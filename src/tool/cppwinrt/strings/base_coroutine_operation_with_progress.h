
namespace std::experimental
{
    template <typename TResult, typename TProgress, typename... Args>
    struct coroutine_traits<winrt::Windows::Foundation::IAsyncOperationWithProgress<TResult, TProgress>, Args...>
    {
        struct promise_type final : winrt::impl::promise_base<promise_type,
            winrt::Windows::Foundation::IAsyncOperationWithProgress<TResult, TProgress>,
            winrt::Windows::Foundation::AsyncOperationWithProgressCompletedHandler<TResult, TProgress>, TProgress>
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
