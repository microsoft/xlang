
namespace winrt::impl
{
    template <typename Async>
    struct async_completed_handler;

    template <typename Async>
    using async_completed_handler_t = typename async_completed_handler<Async>::type;

    template <>
    struct async_completed_handler<Windows::Foundation::IAsyncAction>
    {
        using type = Windows::Foundation::AsyncActionCompletedHandler;
    };

    template <typename TProgress>
    struct async_completed_handler<Windows::Foundation::IAsyncActionWithProgress<TProgress>>
    {
        using type = Windows::Foundation::AsyncActionWithProgressCompletedHandler<TProgress>;
    };

    template <typename TResult>
    struct async_completed_handler<Windows::Foundation::IAsyncOperation<TResult>>
    {
        using type = Windows::Foundation::AsyncOperationCompletedHandler<TResult>;
    };

    template <typename TResult, typename TProgress>
    struct async_completed_handler<Windows::Foundation::IAsyncOperationWithProgress<TResult, TProgress>>
    {
        using type = Windows::Foundation::AsyncOperationWithProgressCompletedHandler<TResult, TProgress>;
    };

    inline bool is_sta() noexcept
    {
        int32_t aptType;
        int32_t aptTypeQualifier;
        return (error_ok == WINRT_CoGetApartmentType(&aptType, &aptTypeQualifier)) && ((aptType == 0 /*APTTYPE_STA*/) || (aptType == 3 /*APTTYPE_MAINSTA*/));
    }

    template <typename Async>
    void wait_for_completed(Async const& async, uint32_t const timeout)
    {
        void* event = check_pointer(WINRT_CreateEventW(nullptr, true, false, nullptr));

        // The delegate is a local to ensure that the event outlives the call to WaitForSingleObject.
        async_completed_handler_t<Async> delegate = [event = handle(event)](auto && ...)
        {
            WINRT_VERIFY(WINRT_SetEvent(event.get()));
        };

        async.Completed(delegate);
        WINRT_WaitForSingleObject(event, timeout);
    }

    template <typename Async>
    auto wait_for(Async const& async, Windows::Foundation::TimeSpan const& timeout)
    {
        WINRT_ASSERT(!is_sta());
        wait_for_completed(async, static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count()));
        return async.Status();
    }

    template <typename Async>
    auto wait_get(Async const& async)
    {
        WINRT_ASSERT(!is_sta());

        if (async.Status() == Windows::Foundation::AsyncStatus::Started)
        {
            wait_for_completed(async, 0xFFFFFFFF); // INFINITE
        }

        return async.GetResults();
    }

    template <typename Async>
    struct await_adapter
    {
        Async const& async;

        bool await_ready() const
        {
            return async.Status() == Windows::Foundation::AsyncStatus::Completed;
        }

        void await_suspend(std::experimental::coroutine_handle<> handle) const
        {
            auto context = capture<IContextCallback>(WINRT_CoGetObjectContext);

            async.Completed([handle, context = std::move(context)](auto const&, Windows::Foundation::AsyncStatus)
            {
                com_callback_args args{};
                args.data = handle.address();

                auto callback = [](com_callback_args* args) noexcept -> int32_t
                {
                    std::experimental::coroutine_handle<>::from_address(args->data)();
                    return error_ok;
                };

                check_hresult(context->ContextCallback(callback, &args, guid_of<impl::ICallbackWithNoReentrancyToApplicationSTA>(), 5, nullptr));
            });
        }

        auto await_resume() const
        {
            return async.GetResults();
        }
    };
}

#ifdef _RESUMABLE_FUNCTIONS_SUPPORTED
namespace winrt::Windows::Foundation
{
    inline impl::await_adapter<IAsyncAction> operator co_await(IAsyncAction const& async)
    {
        return{ async };
    }

    template <typename TProgress>
    impl::await_adapter<IAsyncActionWithProgress<TProgress>> operator co_await(IAsyncActionWithProgress<TProgress> const& async)
    {
        return{ async };
    }

    template <typename TResult>
    impl::await_adapter<IAsyncOperation<TResult>> operator co_await(IAsyncOperation<TResult> const& async)
    {
        return{ async };
    }

    template <typename TResult, typename TProgress>
    impl::await_adapter<IAsyncOperationWithProgress<TResult, TProgress>> operator co_await(IAsyncOperationWithProgress<TResult, TProgress> const& async)
    {
        return{ async };
    }
}
#endif

namespace winrt
{
    struct get_progress_token_t {};

    inline get_progress_token_t get_progress_token() noexcept
    {
        return{};
    }

    struct get_cancellation_token_t {};

    inline get_cancellation_token_t get_cancellation_token() noexcept
    {
        return{};
    }
}

namespace winrt::impl
{
    template <typename Promise>
    struct cancellation_token
    {
        cancellation_token(Promise* promise) noexcept : m_promise(promise)
        {
        }

        bool await_ready() const noexcept
        {
            return true;
        }

        void await_suspend(std::experimental::coroutine_handle<>) const noexcept
        {
        }

        cancellation_token<Promise> await_resume() const noexcept
        {
            return *this;
        }

        bool operator()() const noexcept
        {
            return m_promise->Status() == Windows::Foundation::AsyncStatus::Canceled;
        }

        void callback(winrt::delegate<>&& cancel) noexcept
        {
            m_promise->cancellation_callback(std::move(cancel));
        }

    private:

        Promise* m_promise;
    };

    template <typename Promise, typename Progress>
    struct progress_token
    {
        progress_token(Promise* promise) noexcept :
            m_promise(promise)
        {
        }

        bool await_ready() const noexcept
        {
            return true;
        }

        void await_suspend(std::experimental::coroutine_handle<>) const noexcept
        {
        }

        progress_token<Promise, Progress> await_resume() const noexcept
        {
            return *this;
        }

        void operator()(Progress const& result)
        {
            m_promise->set_progress(result);
        }

    private:

        Promise* m_promise;
    };

    template <typename Derived, typename AsyncInterface, typename TProgress = void>
    struct promise_base : implements<Derived, AsyncInterface, Windows::Foundation::IAsyncInfo>
    {
        using AsyncStatus = Windows::Foundation::AsyncStatus;

        unsigned long WINRT_CALL Release() noexcept
        {
            uint32_t const remaining = this->subtract_reference();

            if (remaining == 0)
            {
                std::atomic_thread_fence(std::memory_order_acquire);
                std::experimental::coroutine_handle<Derived>::from_promise(*static_cast<Derived*>(this)).destroy();
            }

            return remaining;
        }

        void Completed(async_completed_handler_t<AsyncInterface> const& handler)
        {
            AsyncStatus status;

            {
                slim_lock_guard const guard(m_lock);

                if (m_completed_assigned)
                {
                    throw hresult_illegal_delegate_assignment();
                }

                m_completed_assigned = true;

                if (m_status == AsyncStatus::Started)
                {
                    m_completed = make_agile_delegate(handler);
                    return;
                }

                status = m_status;
            }

            if (handler)
            {
                handler(*this, status);
            }
        }

        auto Completed() noexcept
        {
            slim_lock_guard const guard(m_lock);
            return m_completed;
        }

        uint32_t Id() const noexcept
        {
            return 1;
        }

        AsyncStatus Status() noexcept
        {
            slim_lock_guard const guard(m_lock);
            return m_status;
        }

        hresult ErrorCode() noexcept
        {
            try
            {
                slim_lock_guard const guard(m_lock);
                rethrow_if_failed();
                return error_ok;
            }
            catch (...)
            {
                return to_hresult();
            }
        }

        void Cancel() noexcept
        {
            winrt::delegate<> cancel;

            {
                slim_lock_guard const guard(m_lock);

                if (m_status == AsyncStatus::Started)
                {
                    m_status = AsyncStatus::Canceled;
                    m_exception = std::make_exception_ptr(hresult_canceled());
                    cancel = std::move(m_cancel);
                }
            }

            if (cancel)
            {
                cancel();
            }
        }

        void Close() const noexcept
        {
        }

        auto GetResults()
        {
            slim_lock_guard const guard(m_lock);

            if (m_status == AsyncStatus::Completed)
            {
                return static_cast<Derived*>(this)->get_return_value();
            }

            rethrow_if_failed();
            WINRT_ASSERT(m_status == AsyncStatus::Started);
            throw hresult_illegal_method_call();
        }

        AsyncInterface get_return_object() const noexcept
        {
            return *this;
        }

        void get_return_value() const noexcept
        {
        }

        void set_completed() noexcept
        {
            async_completed_handler_t<AsyncInterface> handler;
            AsyncStatus status;

            {
                slim_lock_guard const guard(m_lock);

                if (m_status == AsyncStatus::Started)
                {
                    m_status = AsyncStatus::Completed;
                }

                handler = std::move(this->m_completed);
                status = this->m_status;
            }

            if (handler)
            {
                handler(*this, status);
            }
        }

        std::experimental::suspend_never initial_suspend() const noexcept
        {
            return{};
        }

        struct final_suspend_awaiter
        {
            promise_base* promise;

            bool await_ready() const noexcept
            {
                return false;
            }

            void await_resume() const noexcept
            {
            }

            bool await_suspend(std::experimental::coroutine_handle<>) const noexcept
            {
                promise->set_completed();
                uint32_t const remaining = promise->subtract_reference();

                if (remaining == 0)
                {
                    std::atomic_thread_fence(std::memory_order_acquire);
                }

                return remaining > 0;
            }
        };

        auto final_suspend() noexcept
        {
            return final_suspend_awaiter{ this };
        }

        void unhandled_exception() noexcept
        {
            slim_lock_guard const guard(m_lock);
            WINRT_ASSERT(m_status == AsyncStatus::Started || m_status == AsyncStatus::Canceled);
            m_exception = std::current_exception();

            try
            {
                std::rethrow_exception(m_exception);
            }
            catch (hresult_canceled const&)
            {
                m_status = AsyncStatus::Canceled;
            }
            catch (...)
            {
                m_status = AsyncStatus::Error;
            }
        }

        template <typename Expression>
        Expression&& await_transform(Expression&& expression)
        {
            if (Status() == AsyncStatus::Canceled)
            {
                throw winrt::hresult_canceled();
            }

            return std::forward<Expression>(expression);
        }

        cancellation_token<Derived> await_transform(get_cancellation_token_t) noexcept
        {
            return{ static_cast<Derived*>(this) };
        }

        progress_token<Derived, TProgress> await_transform(get_progress_token_t) noexcept
        {
            return{ static_cast<Derived*>(this) };
        }

        void cancellation_callback(winrt::delegate<>&& cancel) noexcept
        {
            {
                slim_lock_guard const guard(m_lock);

                if (m_status != AsyncStatus::Canceled)
                {
                    m_cancel = std::move(cancel);
                    return;
                }
            }

            cancel();
        }

#if defined(_DEBUG) && !defined(WINRT_NO_MAKE_DETECTION)
        void use_make_function_to_create_this_object() final
        {
        }
#endif

    protected:

        void rethrow_if_failed() const
        {
            if (m_status == AsyncStatus::Error || m_status == AsyncStatus::Canceled)
            {
                std::rethrow_exception(m_exception);
            }
        }

        std::exception_ptr m_exception{};
        slim_mutex m_lock;
        async_completed_handler_t<AsyncInterface> m_completed;
        winrt::delegate<> m_cancel;
        AsyncStatus m_status{ AsyncStatus::Started };
        bool m_completed_assigned{ false };
    };
}

namespace std::experimental
{
    template <typename... Args>
    struct coroutine_traits<winrt::Windows::Foundation::IAsyncAction, Args...>
    {
        struct promise_type final : winrt::impl::promise_base<promise_type, winrt::Windows::Foundation::IAsyncAction>
        {
            void return_void() const noexcept
            {
            }
        };
    };

    template <typename TProgress, typename... Args>
    struct coroutine_traits<winrt::Windows::Foundation::IAsyncActionWithProgress<TProgress>, Args...>
    {
        struct promise_type final : winrt::impl::promise_base<promise_type, winrt::Windows::Foundation::IAsyncActionWithProgress<TProgress>, TProgress>
        {
            using ProgressHandler = winrt::Windows::Foundation::AsyncActionProgressHandler<TProgress>;

            void Progress(ProgressHandler const& handler) noexcept
            {
                winrt::slim_lock_guard const guard(this->m_lock);
                m_progress = winrt::impl::make_agile_delegate(handler);
            }

            ProgressHandler Progress() noexcept
            {
                winrt::slim_lock_guard const guard(this->m_lock);
                return m_progress;
            }

            void return_void() const noexcept
            {
            }

            void set_progress(TProgress const& result)
            {
                if (auto handler = Progress())
                {
                    handler(*this, result);
                }
            }

            ProgressHandler m_progress;
        };
    };

    template <typename TResult, typename... Args>
    struct coroutine_traits<winrt::Windows::Foundation::IAsyncOperation<TResult>, Args...>
    {
        struct promise_type final : winrt::impl::promise_base<promise_type, winrt::Windows::Foundation::IAsyncOperation<TResult>>
        {
            TResult get_return_value() noexcept
            {
                return std::move(m_result);
            }

            void return_value(TResult&& value) noexcept
            {
                m_result = std::move(value);
            }

            void return_value(TResult const& value) noexcept
            {
                m_result = value;
            }

            TResult m_result{ winrt::impl::empty_value<TResult>() };
        };
    };

    template <typename TResult, typename TProgress, typename... Args>
    struct coroutine_traits<winrt::Windows::Foundation::IAsyncOperationWithProgress<TResult, TProgress>, Args...>
    {
        struct promise_type final : winrt::impl::promise_base<promise_type,
            winrt::Windows::Foundation::IAsyncOperationWithProgress<TResult, TProgress>, TProgress>
        {
            using ProgressHandler = winrt::Windows::Foundation::AsyncOperationProgressHandler<TResult, TProgress>;

            void Progress(ProgressHandler const& handler) noexcept
            {
                winrt::slim_lock_guard const guard(this->m_lock);
                m_progress = winrt::impl::make_agile_delegate(handler);
            }

            ProgressHandler Progress() noexcept
            {
                winrt::slim_lock_guard const guard(this->m_lock);
                return m_progress;
            }

            TResult get_return_value() noexcept
            {
                return std::move(m_result);
            }

            void return_value(TResult&& value) noexcept
            {
                m_result = std::move(value);
            }

            void return_value(TResult const& value) noexcept
            {
                m_result = value;
            }

            void set_progress(TProgress const& result)
            {
                if (auto handler = Progress())
                {
                    handler(*this, result);
                }
            }

            TResult m_result{ winrt::impl::empty_value<TResult>() };
            ProgressHandler m_progress;
        };
    };
}
