#pragma once

#include "pal.h"
#include <string_view>

namespace xlang::impl
{
    xlang_pfn_lib_get_activation_factory try_get_activation_func(
        std::basic_string_view<char> module_name);

    xlang_pfn_lib_get_activation_factory try_get_activation_func(
        std::basic_string_view<wchar_t> module_name);
}
