
namespace std::experimental
{
    template <typename TProgress, typename... Args>
    struct coroutine_traits<xlang::Windows::Foundation::IAsyncActionWithProgress<TProgress>, Args...>
    {
        struct promise_type final : xlang::impl::promise_base<promise_type,
            xlang::Windows::Foundation::IAsyncActionWithProgress<TProgress>,
            xlang::Windows::Foundation::AsyncActionWithProgressCompletedHandler<TProgress>, TProgress>
        {
            using ProgressHandler = xlang::Windows::Foundation::AsyncActionProgressHandler<TProgress>;

            void Progress(ProgressHandler const& handler) noexcept
            {
                std::lock_guard const guard(this->m_lock);
                m_progress = handler;
            }

            ProgressHandler Progress() noexcept
            {
                std::lock_guard const guard(this->m_lock);
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
