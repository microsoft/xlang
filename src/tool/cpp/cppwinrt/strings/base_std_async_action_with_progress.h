
WINRT_EXPORT namespace std::experimental
{
    template <typename TProgress, typename... Args>
    struct coroutine_traits<winrt::Windows::Foundation::IAsyncActionWithProgress<TProgress>, Args...>
    {
        struct promise_type final : winrt::impl::promise_base<promise_type,
            winrt::Windows::Foundation::IAsyncActionWithProgress<TProgress>,
            winrt::Windows::Foundation::AsyncActionWithProgressCompletedHandler<TProgress>, TProgress>
        {
            using ProgressHandler = winrt::Windows::Foundation::AsyncActionProgressHandler<TProgress>;

            void Progress(ProgressHandler const& handler) noexcept
            {
                winrt::slim_lock_guard const guard(this->m_lock);
                m_progress = handler;
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
}
