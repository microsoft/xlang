
WINRT_EXPORT namespace std::experimental
{
    template <typename... Args>
    struct coroutine_traits<xlang::System::IAsyncAction, Args...>
    {
        struct promise_type final : xlang::impl::promise_base<promise_type, xlang::System::IAsyncAction,
            xlang::System::AsyncActionCompletedHandler>
        {
            using AsyncStatus = xlang::System::AsyncStatus;

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
                xlang::System::AsyncActionCompletedHandler handler;
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
        };
    };
}
