
WINRT_EXPORT namespace std::experimental
{
    template <typename... Args>
    struct coroutine_traits<winrt::Windows::Foundation::IAsyncAction, Args...>
    {
        struct promise_type final : winrt::impl::promise_base<promise_type, winrt::Windows::Foundation::IAsyncAction,
            winrt::Windows::Foundation::AsyncActionCompletedHandler>
        {
            using AsyncStatus = winrt::Windows::Foundation::AsyncStatus;

            void return_void()
            {
                winrt::Windows::Foundation::AsyncActionCompletedHandler handler;
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
        };
    };
}
