
WINRT_EXPORT namespace std::experimental
{
    template <typename TResult, typename TProgress, typename... Args>
    struct coroutine_traits<xlang::IAsyncOperationWithProgress<TResult, TProgress>, Args...>
    {
        struct promise_type final : xlang::impl::promise_base<promise_type, xlang::IAsyncOperationWithProgress<TResult, TProgress>,
            xlang::AsyncOperationWithProgressCompletedHandler<TResult, TProgress>, TProgress>
        {
            using AsyncStatus = xlang::AsyncStatus;
            using ProgressHandler = xlang::AsyncOperationProgressHandler<TResult, TProgress>;

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

            TResult GetResults()
            {
                xlang::slim_lock_guard const guard(this->m_lock);

                if (this->m_status == AsyncStatus::Completed)
                {
                    return m_result;
                }

                this->rethrow_if_failed();
                WINRT_ASSERT(this->m_status == AsyncStatus::Started);
                throw xlang::hresult_illegal_method_call();
            }

            void return_value(TResult const& result)
            {
                xlang::AsyncOperationWithProgressCompletedHandler<TResult, TProgress> handler;
                AsyncStatus status;

                {
                    xlang::slim_lock_guard const guard(this->m_lock);

                    if (this->m_status == AsyncStatus::Started)
                    {
                        this->m_status = AsyncStatus::Completed;
                        m_result = result;
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

            TResult m_result{ xlang::impl::empty_value<TResult>() };
            ProgressHandler m_progress;
        };
    };
}
