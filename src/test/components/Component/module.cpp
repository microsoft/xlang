#include <winrt/Component.h>
#include <pal.h>

XLANG_EXPORT_DECL xlang_result xlang_lib_get_activation_factory(xlang_string class_name, xlang_guid const& iid, void **factory) noexcept
{
    *factory = nullptr;
    char16_t const* buffer{};
    uint32_t length{};

    xlang_result result = xlang_get_string_raw_buffer_utf16(class_name, &buffer, *length);
    if (result != xlang_error_ok)
    {
        return result;
    }
}