
WINRT_EXPORT namespace std::experimental
{
    template <typename TProgress, typename... Args>
    struct coroutine_traits<xlang::Runtime::IAsyncActionWithProgress<TProgress>, Args...>
    {
        struct promise_type final : xlang::impl::promise_base<promise_type, xlang::Runtime::IAsyncActionWithProgress<TProgress>,
            xlang::Runtime::AsyncActionWithProgressCompletedHandler<TProgress>, TProgress>
        {
            using AsyncStatus = xlang::Runtime::AsyncStatus;
            using ProgressHandler = xlang::Runtime::AsyncActionProgressHandler<TProgress>;

            void Progress(ProgressHandler const& handler)
            {
                xlang::slim_lock_guard const guard(this->m_lock);
                m_progress = handler;
            }

            ProgressHandler Progress()
            {
                xlang::slim_lock_guard const guard(this->m_lock);
                return m_progress;
            }

            void GetResults()
            {
                xlang::slim_lock_guard const guard(this->m_lock);

                if (this->m_status == AsyncStatus::Completed)
                {
                    return;
                }

                this->rethrow_if_failed();
                WINRT_ASSERT(this->m_status == AsyncStatus::Started);
                throw xlang::hresult_illegal_method_call();
            }

            void return_void()
            {
                xlang::Runtime::AsyncActionWithProgressCompletedHandler<TProgress> handler;
                AsyncStatus status;

                {
                    xlang::slim_lock_guard const guard(this->m_lock);

                    if (this->m_status == AsyncStatus::Started)
                    {
                        this->m_status = AsyncStatus::Completed;
                    }
                    else
                    {
                        WINRT_ASSERT(this->m_status == AsyncStatus::Canceled);
                        this->m_exception = make_exception_ptr(xlang::hresult_canceled());
                    }

                    handler = std::move(this->m_completed);
                    status = this->m_status;
                }

                if (handler)
                {
                    handler(*this, status);
                }
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
}
