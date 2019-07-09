
namespace xlang::impl
{
#ifdef __IUnknown_INTERFACE_DEFINED__
#define XLANG_WINDOWS_ABI
    using hresult_type = long;
    using ref_count_type = unsigned long;
#else
    using hresult_type = int32_t;
    using ref_count_type = uint32_t;
#endif

    using ptp_io = struct tp_io*;
    using ptp_timer = struct tp_timer*;
    using ptp_wait = struct tp_wait*;
    using bstr = wchar_t*;
}

namespace xlang
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

	using guid = xlang_guid;
}

namespace xlang::Windows::Foundation
{
    enum class TrustLevel : int32_t
    {
        BaseTrust,
        PartialTrust,
        FullTrust
    };

    struct IUnknown;
    struct IXlangObject;
    struct IActivationFactory;
}
