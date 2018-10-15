#include "pal_internal.h"
#include "heap_string.h"

namespace xlang::impl
{
    // Provide conversions between opaque string types and xlang_string.
    // Put another way, insulate opaque_string.h from the public opaque handle types like xlang_string.
    inline string_base* from_handle(xlang_string handle) noexcept
    {
        return reinterpret_cast<string_base*>(handle);
    }

    inline heap_string* from_handle(xlang_string_buffer handle) noexcept
    {
        return reinterpret_cast<heap_string*>(handle);
    }

    inline xlang_string to_handle(string_base* str) noexcept
    {
        return reinterpret_cast<xlang_string>(str);
    }

    inline xlang_string_buffer to_buffer_handle(heap_string* str) noexcept
    {
        return reinterpret_cast<xlang_string_buffer>(str);
    }

    template <typename char_type>
    inline std::basic_string_view<char_type> to_string_view(xlang_string handle)
    {
        return from_handle(handle)->ensure_buffer<char_type>();
    }

#ifdef XLANG_PLATFORM_WINDOWS
    template <>
    inline std::basic_string_view<wchar_t> to_string_view<wchar_t>(xlang_string handle)
    {
        auto const temp{ to_string_view<char16_t>(handle) };
        return { reinterpret_cast<wchar_t const*>(temp.data()), temp.size() };
    }
#endif
}