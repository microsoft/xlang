
namespace winrt::impl
{
    template <typename T>
    auto detach_from(T&& object) noexcept
    {
        return detach_abi(std::forward<T>(object));
    }
}
