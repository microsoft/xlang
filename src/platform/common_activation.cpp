#include "pal_internal.h"
#include "platform_activation.h"
#include "string_convert.h"
#include "pal_error.h"

namespace xlang::impl
{
    xlang_pfn_lib_get_activation_factory try_get_activation_func(
        std::basic_string_view<char16_t> module_namespace)
    {
        // TODO: implement try_get_activation_func for non-windows platforms
        throw_result(xlang_error_sadness);
    }

    xlang_pfn_lib_get_activation_factory try_get_activation_func(
        std::basic_string_view<char> module_namespace)
    {
        // TODO: implement try_get_activation_func for non-windows platforms
        throw_result(xlang_error_sadness);
    }

}