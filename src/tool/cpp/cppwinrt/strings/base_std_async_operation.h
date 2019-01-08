
WINRT_EXPORT namespace std::experimental
{
    template <typename TResult, typename... Args>
    struct coroutine_traits<winrt::Windows::Foundation::IAsyncOperation<TResult>, Args...>
    {
        struct promise_type final : winrt::impl::promise_base<promise_type,
            winrt::Windows::Foundation::IAsyncOperation<TResult>,
            winrt::Windows::Foundation::AsyncOperationCompletedHandler<TResult>>
        {
            TResult get_return_value() const noexcept
            {
                return m_result;
            }

            void return_value(TResult const& result) noexcept
            {
                m_result = result;
            }

            TResult m_result{ winrt::impl::empty_value<TResult>() };
        };
    };
}
