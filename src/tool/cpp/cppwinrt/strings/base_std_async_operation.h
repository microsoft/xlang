
WINRT_EXPORT namespace std::experimental
{
    template <typename TResult, typename... Args>
    struct coroutine_traits<winrt::Windows::Foundation::IAsyncOperation<TResult>, Args...>
    {
        struct promise_type final : winrt::impl::promise_base<promise_type,
            winrt::Windows::Foundation::IAsyncOperation<TResult>,
            winrt::Windows::Foundation::AsyncOperationCompletedHandler<TResult>>
        {
            using AsyncStatus = winrt::Windows::Foundation::AsyncStatus;

            TResult get_return_value() const
            {
                return m_result;
            }

            void return_value(TResult const& result)
            {
                m_result = result;
                this->set_completed();
            }

            TResult m_result{ winrt::impl::empty_value<TResult>() };
        };
    };
}
