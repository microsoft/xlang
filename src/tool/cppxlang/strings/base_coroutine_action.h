
namespace std::experimental
{
    template <typename... Args>
    struct coroutine_traits<xlang::Windows::Foundation::IAsyncAction, Args...>
    {
        struct promise_type final : xlang::impl::promise_base<promise_type,
            xlang::Windows::Foundation::IAsyncAction,
            xlang::Windows::Foundation::AsyncActionCompletedHandler>
        {
            void return_void() const noexcept
            {
            }
        };
    };
}
