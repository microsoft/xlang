
namespace xlang::impl
{
    template <typename Async>
    void blocking_suspend(Async const& async)
    {
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
            return async.Status() == AsyncStatus::Completed;
        }

        void await_suspend(std::experimental::coroutine_handle<> handle) const
        {
            async.Completed([handle](auto&&...)
            {
                handle();
            });
        }

        auto await_resume() const
        {
            return async.GetResults();
        }
    };
}

#ifdef _RESUMABLE_FUNCTIONS_SUPPORTED
WINRT_EXPORT namespace xlang
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

WINRT_EXPORT namespace xlang
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

    struct fire_and_forget {};
}

namespace xlang::impl
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
            return m_promise->Status() == AsyncStatus::Canceled;
        }

        void callback(xlang::delegate<>&& cancel) noexcept
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
    struct promise_base : implements<Derived, AsyncInterface>
    {
        using AsyncStatus = AsyncStatus;

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
                    m_completed = handler;
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
            xlang::delegate<> cancel;

            {
                slim_lock_guard const guard(m_lock);

                if (m_status == AsyncStatus::Started)
                {
                    m_status = AsyncStatus::Canceled;
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

        AsyncInterface get_return_object() const noexcept
        {
            return*this;
        }

        std::experimental::suspend_never initial_suspend() const noexcept
        {
            return{};
        }

        struct final_suspend_type
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
                uint32_t const remaining = promise->subtract_reference();

                if (remaining == 0)
                {
                    std::atomic_thread_fence(std::memory_order_acquire);
                }

                return remaining > 0;
            }
        };

        final_suspend_type final_suspend() noexcept
        {
            return{ this };
        }

        void unhandled_exception() noexcept
        {
            CompletedHandler handler;
            AsyncStatus status;

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

                handler = std::move(m_completed);
                status = m_status;
            }

            if (handler)
            {
                handler(*this, status);
            }
        }

        template <typename Expression>
        Expression&& await_transform(Expression&& expression)
        {
            if (Status() == AsyncStatus::Canceled)
            {
                throw xlang::hresult_canceled();
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

        void cancellation_callback(xlang::delegate<>&& cancel) noexcept
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
        xlang::delegate<> m_cancel;
        AsyncStatus m_status{ AsyncStatus::Started };
        bool m_completed_assigned{ false };
    };
}
