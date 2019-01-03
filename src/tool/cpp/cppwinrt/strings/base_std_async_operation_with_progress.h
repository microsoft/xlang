
WINRT_EXPORT namespace std::experimental
{
    template <typename TResult, typename TProgress, typename... Args>
    struct coroutine_traits<winrt::Windows::Foundation::IAsyncOperationWithProgress<TResult, TProgress>, Args...>
    {
        struct promise_type final : winrt::impl::promise_base<promise_type,
            winrt::Windows::Foundation::IAsyncOperationWithProgress<TResult, TProgress>,
            winrt::Windows::Foundation::AsyncOperationWithProgressCompletedHandler<TResult, TProgress>, TProgress>
        {
            using ProgressHandler = winrt::Windows::Foundation::AsyncOperationProgressHandler<TResult, TProgress>;

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

            TResult get_return_value() const
            {
                return m_result;
            }

            void return_value(TResult const& result)
            {
                m_result = result;
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
