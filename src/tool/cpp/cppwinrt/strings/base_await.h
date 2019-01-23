
namespace winrt::impl
{
    inline bool is_sta() noexcept
    {
        int32_t aptType;
        int32_t aptTypeQualifier;
        return (error_ok == WINRT_CoGetApartmentType(&aptType, &aptTypeQualifier)) && ((aptType == 0 /*APTTYPE_STA*/) || (aptType == 3 /*APTTYPE_MAINSTA*/));
    }

    template <typename Async>
    void blocking_suspend(Async const& async)
    {
        WINRT_ASSERT(!is_sta());

        slim_mutex m;
        slim_condition_variable cv;
        bool completed = false;
        async.Completed([&](auto&&...)
        {
            {
                slim_lock_guard const guard(m);
                completed = true;
            }
            cv.notify_one();
        });

        slim_lock_guard guard(m);
        cv.wait(m, [&] { return completed; });
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
    inline auto resume_background()
    {
        struct awaitable
        {
            bool await_ready() const noexcept
            {
                return false;
            }

            void await_resume() const noexcept
            {
            }

            void await_suspend(std::experimental::coroutine_handle<> handle) const
            {
                if (!WINRT_TrySubmitThreadpoolCallback(callback, handle.address(), nullptr))
                {
                    throw_last_error();
                }
            }

        private:

            static void WINRT_CALL callback(void*, void* context) noexcept
            {
                std::experimental::coroutine_handle<>::from_address(context)();
            }
        };

        return awaitable{};
    }

    template <typename T>
    auto resume_background(T&& context)
    {
        struct awaitable
        {
            awaitable(T&& context) : m_context(std::forward<T>(context))
            {
            }

            bool await_ready() const noexcept
            {
                return false;
            }

            void await_resume() const noexcept
            {
            }

            void await_suspend(std::experimental::coroutine_handle<> resume)
            {
                m_resume = resume;

                if (!WINRT_TrySubmitThreadpoolCallback(callback, this, nullptr))
                {
                    throw_last_error();
                }
            }

        private:

            static void WINRT_CALL callback(void*, void* context) noexcept
            {
                auto that = static_cast<awaitable*>(context);
                auto guard = that->m_context();
                that->m_resume();
            }

            T&& m_context;
            std::experimental::coroutine_handle<> m_resume{ nullptr };
        };

        return awaitable{ std::forward<T>(context) };
    }

    struct apartment_context
    {
        apartment_context()
        {
            m_context.capture(WINRT_CoGetObjectContext);
        }

        bool await_ready() const noexcept
        {
            return false;
        }

        void await_resume() const noexcept
        {
        }

        void await_suspend(std::experimental::coroutine_handle<> handle) const
        {
            impl::com_callback_args args{};
            args.data = handle.address();
            check_hresult(m_context->ContextCallback(callback, &args, guid_of<impl::ICallbackWithNoReentrancyToApplicationSTA>(), 5, nullptr));
        }

    private:

        static int32_t WINRT_CALL callback(impl::com_callback_args* args) noexcept
        {
            std::experimental::coroutine_handle<>::from_address(args->data)();
            return impl::error_ok;
        }

        com_ptr<impl::IContextCallback> m_context;
    };

    struct resume_after
    {
        explicit resume_after(Windows::Foundation::TimeSpan duration) noexcept : m_duration(duration)
        {
        }

        bool await_ready() const noexcept
        {
            return m_duration.count() <= 0;
        }

        void await_suspend(std::experimental::coroutine_handle<> handle)
        {
            m_timer.attach(check_pointer(WINRT_CreateThreadpoolTimer(callback, handle.address(), nullptr)));
            int64_t relative_count = -m_duration.count();
            WINRT_SetThreadpoolTimer(m_timer.get(), &relative_count, 0, 0);
        }

        void await_resume() const noexcept
        {
        }

    private:

        static void WINRT_CALL callback(void*, void* context, void*) noexcept
        {
            std::experimental::coroutine_handle<>::from_address(context)();
        }

        struct timer_traits
        {
            using type = impl::ptp_timer;

            static void close(type value) noexcept
            {
                WINRT_CloseThreadpoolTimer(value);
            }

            static constexpr type invalid() noexcept
            {
                return nullptr;
            }
        };

        handle_type<timer_traits> m_timer;
        Windows::Foundation::TimeSpan m_duration;
    };

    struct resume_on_signal
    {
        explicit resume_on_signal(void* handle) noexcept :
            m_handle(handle)
        {}

        resume_on_signal(void* handle, Windows::Foundation::TimeSpan timeout) noexcept :
            m_timeout(timeout),
            m_handle(handle)
        {}

        bool await_ready() const noexcept
        {
            return WINRT_WaitForSingleObject(m_handle, 0) == 0;
        }

        void await_suspend(std::experimental::coroutine_handle<> resume)
        {
            m_resume = resume;
            m_wait.attach(check_pointer(WINRT_CreateThreadpoolWait(callback, this, nullptr)));
            int64_t relative_count = -m_timeout.count();
            int64_t* file_time = relative_count != 0 ? &relative_count : nullptr;
            WINRT_SetThreadpoolWait(m_wait.get(), m_handle, file_time);
        }

        bool await_resume() const noexcept
        {
            return m_result == 0;
        }

    private:

        static void WINRT_CALL callback(void*, void* context, void*, uint32_t result) noexcept
        {
            auto that = static_cast<resume_on_signal*>(context);
            that->m_result = result;
            that->m_resume();
        }

        struct wait_traits
        {
            using type = impl::ptp_wait;

            static void close(type value) noexcept
            {
                WINRT_CloseThreadpoolWait(value);
            }

            static constexpr type invalid() noexcept
            {
                return nullptr;
            }
        };

        handle_type<wait_traits> m_wait;
        Windows::Foundation::TimeSpan m_timeout{ 0 };
        void* m_handle{};
        uint32_t m_result{};
        std::experimental::coroutine_handle<> m_resume{ nullptr };
    };

    struct overlapped_io
    {
        uintptr_t Internal;
        uintptr_t InternalHigh;
        union
        {
            struct
            {
                uint32_t Offset;
                uint32_t OffsetHigh;
            } s;
            void* Pointer;
        };
        void* hEvent;
    };

    struct awaitable_base
    {
        static void WINRT_CALL callback(void*, void*, void* overlapped, uint32_t result, std::size_t, void*) noexcept
        {
            auto context = static_cast<awaitable_base*>(overlapped);
            context->m_result = result;
            context->m_resume();
        }

    protected:

        overlapped_io m_overlapped{};
        uint32_t m_result{};
        std::experimental::coroutine_handle<> m_resume{ nullptr };
    };

    struct resumable_io
    {
        resumable_io(void* object) :
            m_io(check_pointer(WINRT_CreateThreadpoolIo(object, awaitable_base::callback, nullptr, nullptr)))
        {
        }

        template <typename F>
        auto start(F callback)
        {
            struct awaitable : awaitable_base, F
            {
                awaitable(impl::ptp_io io, F callback) noexcept :
                    F(callback),
                    m_io(io)
                {}

                bool await_ready() const noexcept
                {
                    return false;
                }

                void await_suspend(std::experimental::coroutine_handle<> resume_handle)
                {
                    m_resume = resume_handle;
                    WINRT_StartThreadpoolIo(m_io);

                    try
                    {
                        (*this)(m_overlapped);
                    }
                    catch (...)
                    {
                        WINRT_CancelThreadpoolIo(m_io);
                        throw;
                    }
                }

                uint32_t await_resume() const
                {
                    if (m_result != 38 /*ERROR_HANDLE_EOF*/)
                    {
                        check_win32(m_result);
                    }
                    return static_cast<uint32_t>(m_overlapped.InternalHigh);
                }

                impl::ptp_io m_io{};
            };

            return awaitable(get(), callback);
        }

        template <typename F>
        auto start_pending(F callback)
        {
            struct awaitable : awaitable_base, F
            {
                awaitable(impl::ptp_io io, F callback) noexcept :
                    F(callback),
                    m_io(io)
                {}

                bool await_ready() const noexcept
                {
                    return false;
                }

                bool await_suspend(std::experimental::coroutine_handle<> resume_handle)
                {
                    m_resume = resume_handle;
                    WINRT_StartThreadpoolIo(m_io);

                    try
                    {
                        bool const pending = (*this)(m_overlapped);

                        if (!pending)
                        {
                            WINRT_CancelThreadpoolIo(m_io);
                        }

                        return pending;
                    }
                    catch (...)
                    {
                        WINRT_CancelThreadpoolIo(m_io);
                        throw;
                    }
                }

                uint32_t await_resume() const
                {
                    if (m_result != 38 /*ERROR_HANDLE_EOF*/)
                    {
                        check_win32(m_result);
                    }
                    return static_cast<uint32_t>(m_overlapped.InternalHigh);
                }

                impl::ptp_io m_io{};
            };

            return awaitable(get(), callback);
        }

        impl::ptp_io get() const noexcept
        {
            return m_io.get();
        }

    private:

        struct io_traits
        {
            using type = impl::ptp_io;

            static void close(type value) noexcept
            {
                WINRT_CloseThreadpoolIo(value);
            }

            static constexpr type invalid() noexcept
            {
                return nullptr;
            }
        };

        handle_type<io_traits> m_io;
    };

#ifdef _RESUMABLE_FUNCTIONS_SUPPORTED
    inline auto operator co_await(Windows::Foundation::TimeSpan duration)
    {
        return resume_after(duration);
    }
#endif

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
            return*this;
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

        Promise * m_promise;
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
            return*this;
        }

        void operator()(Progress const& result)
        {
            m_promise->set_progress(result);
        }

    private:

        Promise * m_promise;
    };

    template <typename Derived, typename AsyncInterface, typename CompletedHandler, typename TProgress = void>
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

        void Completed(CompletedHandler const& handler)
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

        CompletedHandler Completed() noexcept
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
            return*this;
        }

        void get_return_value() const noexcept
        {
        }

        void set_completed() noexcept
        {
            CompletedHandler handler;
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

        auto final_suspend() noexcept
        {
            struct awaiter
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

            return awaiter{ this };
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
        CompletedHandler m_completed;
        winrt::delegate<> m_cancel;
        AsyncStatus m_status{ AsyncStatus::Started };
        bool m_completed_assigned{ false };
    };
}
