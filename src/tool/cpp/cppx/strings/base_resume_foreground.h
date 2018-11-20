
WINRT_EXPORT namespace winrt
{
    struct resume_foreground
    {
        explicit resume_foreground(Windows::UI::Core::CoreDispatcher&& dispatcher, Windows::UI::Core::CoreDispatcherPriority const priority = Windows::UI::Core::CoreDispatcherPriority::Normal) :
            m_dispatcher(std::move(dispatcher)),
            m_priority(priority)
        {
        }

        explicit resume_foreground(Windows::UI::Core::CoreDispatcher const& dispatcher, Windows::UI::Core::CoreDispatcherPriority const priority = Windows::UI::Core::CoreDispatcherPriority::Normal) :
            m_dispatcher(dispatcher),
            m_priority(priority)
        {
        }

        bool await_ready() const
        {
            return m_dispatcher.HasThreadAccess();
        }

        void await_resume() const noexcept
        {
        }

        void await_suspend(std::experimental::coroutine_handle<> handle) const
        {
            m_dispatcher.RunAsync(m_priority, [handle]
            {
                handle();
            });
        }

    private:

        Windows::UI::Core::CoreDispatcher const m_dispatcher;
        Windows::UI::Core::CoreDispatcherPriority const m_priority;
    };
}
