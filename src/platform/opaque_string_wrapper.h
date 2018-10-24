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
}