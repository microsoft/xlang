
WINRT_EXPORT namespace winrt::Windows::Foundation
{
    enum class TrustLevel : int32_t
    {
        BaseTrust,
        PartialTrust,
        FullTrust
    };

    struct IUnknown;
    struct IInspectable;
    struct IActivationFactory;
}

WINRT_EXPORT namespace winrt
{
    struct hresult
    {
        int32_t value{};

        constexpr hresult() noexcept = default;

        constexpr hresult(int32_t const value) noexcept : value(value)
        {
        }

        constexpr operator int32_t() const noexcept
        {
            return value;
        }
    };

    void check_hresult(hresult const result);
    hresult to_hresult() noexcept;

    struct take_ownership_from_abi_t {};
    constexpr take_ownership_from_abi_t take_ownership_from_abi{};

    template <typename T>
    struct com_ptr;
}

namespace winrt::impl
{
    template <typename T>
    struct reference_traits;
}
