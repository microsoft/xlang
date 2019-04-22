
namespace winrt::impl
{
#ifdef __IUnknown_INTERFACE_DEFINED__
#define WINRT_WINDOWS_ABI
    using hresult_type = long;
    using ref_count_type = unsigned long;
#else
    using hresult_type = int32_t;
    using ref_count_type = uint32_t;
#endif

    using ptp_io = struct tp_io*;
    using ptp_timer = struct tp_timer*;
    using ptp_wait = struct tp_wait*;
    using ptp_pool = struct tp_pool*;
    using srwlock = struct srwlock_*;
    using condition_variable = struct condition_variable_*;
    using bstr = wchar_t*;

    inline bool is_guid_equal(uint32_t const* const left, uint32_t const* const right) noexcept
    {
        return left[0] == right[0] && left[1] == right[1] && left[2] == right[2] && left[3] == right[3];
    }
}

namespace winrt
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

    struct guid
    {
        uint32_t Data1;
        uint16_t Data2;
        uint16_t Data3;
        uint8_t  Data4[8];

        guid() noexcept = default;

        constexpr guid(uint32_t const Data1, uint16_t const Data2, uint16_t const Data3, std::array<uint8_t, 8> const& Data4) noexcept :
            Data1(Data1),
            Data2(Data2),
            Data3(Data3),
            Data4{ Data4[0], Data4[1], Data4[2], Data4[3], Data4[4], Data4[5], Data4[6], Data4[7] }
        {
        }

#ifdef WINRT_WINDOWS_ABI

        constexpr guid(GUID const& value) noexcept :
            Data1(value.Data1),
            Data2(value.Data2),
            Data3(value.Data3),
            Data4{ value.Data4[0], value.Data4[1], value.Data4[2], value.Data4[3], value.Data4[4], value.Data4[5], value.Data4[6], value.Data4[7] }
        {

        }

        operator GUID const&() const noexcept
        {
            return reinterpret_cast<GUID const&>(*this);
        }

#endif
    };

    inline bool operator==(guid const& left, guid const& right) noexcept
    {
        return impl::is_guid_equal(reinterpret_cast<uint32_t const*>(&left), reinterpret_cast<uint32_t const*>(&right));
    }

    inline bool operator!=(guid const& left, guid const& right) noexcept
    {
        return !(left == right);
    }
}

namespace winrt::Windows::Foundation
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
