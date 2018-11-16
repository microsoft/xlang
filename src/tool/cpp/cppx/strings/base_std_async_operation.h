
WINRT_EXPORT namespace std::experimental
{
    template <typename TResult, typename... Args>
    struct coroutine_traits<xlang::Windows::Foundation::IAsyncOperation<TResult>, Args...>
    {
        struct promise_type final : xlang::impl::promise_base<promise_type, xlang::Windows::Foundation::IAsyncOperation<TResult>,
            xlang::Windows::Foundation::AsyncOperationCompletedHandler<TResult>>
        {
            using AsyncStatus = xlang::Windows::Foundation::AsyncStatus;

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
                xlang::Windows::Foundation::AsyncOperationCompletedHandler<TResult> handler;
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

            TResult m_result{ xlang::impl::empty_value<TResult>() };
        };
    };
}
