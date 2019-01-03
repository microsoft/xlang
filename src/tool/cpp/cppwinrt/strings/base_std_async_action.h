
WINRT_EXPORT namespace std::experimental
{
    template <typename... Args>
    struct coroutine_traits<winrt::Windows::Foundation::IAsyncAction, Args...>
    {
        struct promise_type final : winrt::impl::promise_base<promise_type,
            winrt::Windows::Foundation::IAsyncAction,
            winrt::Windows::Foundation::AsyncActionCompletedHandler>
        {
            void return_void() const noexcept
            {
            }
        };
    };
}
