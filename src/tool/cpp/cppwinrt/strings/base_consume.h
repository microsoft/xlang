
namespace winrt::impl
{
    template <typename Async>
    void blocking_suspend(Async const& async);

    template <typename D, typename T> struct consume_IReference
    {
        T Value() const
        {
            T result{};
            check_hresult(WINRT_SHIM(Windows::Foundation::IReference<T>)->get_Value(put_abi(result)));
            return result;
        }
    };

    template <typename D, typename T> struct consume_IReferenceArray
    {
        com_array<T> Value() const
        {
            com_array<T> result{};
            check_hresult(WINRT_SHIM(Windows::Foundation::IReferenceArray<T>)->get_Value(impl::put_size_abi(result), put_abi(result)));
            return result;
        }
    };
}
