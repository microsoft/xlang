
WINRT_EXPORT namespace std::experimental
{
    template <typename TProgress, typename... Args>
    struct coroutine_traits<winrt::Windows::Foundation::IAsyncActionWithProgress<TProgress>, Args...>
    {
        struct promise_type final : winrt::impl::promise_base<promise_type, winrt::Windows::Foundation::IAsyncActionWithProgress<TProgress>,
            winrt::Windows::Foundation::AsyncActionWithProgressCompletedHandler<TProgress>, TProgress>
        {
            using AsyncStatus = winrt::Windows::Foundation::AsyncStatus;
            using ProgressHandler = winrt::Windows::Foundation::AsyncActionProgressHandler<TProgress>;

            void Progress(ProgressHandler const& handler)
            {
                winrt::slim_lock_guard const guard(this->m_lock);
                m_progress = handler;
            }

            ProgressHandler Progress()
            {
                winrt::slim_lock_guard const guard(this->m_lock);
                return m_progress;
            }

            void GetResults()
            {
                winrt::slim_lock_guard const guard(this->m_lock);

                if (this->m_status == AsyncStatus::Completed)
                {
                    return;
                }

                this->rethrow_if_failed();
                WINRT_ASSERT(this->m_status == AsyncStatus::Started);
                throw winrt::hresult_illegal_method_call();
            }

            void return_void()
            {
                winrt::Windows::Foundation::AsyncActionWithProgressCompletedHandler<TProgress> handler;
                AsyncStatus status;

                {
                    winrt::slim_lock_guard const guard(this->m_lock);

                    if (this->m_status == AsyncStatus::Started)
                    {
                        this->m_status = AsyncStatus::Completed;
                    }
                    else
                    {
                        WINRT_ASSERT(this->m_status == AsyncStatus::Canceled);
                        this->m_exception = make_exception_ptr(winrt::hresult_canceled());
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
