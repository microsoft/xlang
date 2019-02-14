#pragma once

#include <stdint.h>

namespace xlang::impl
{
    template <typename char_type>
    struct alternate_type;

    template <>
    struct alternate_type<xlang_char8>
    {
        using result_type = char16_t;
    };

    template <>
    struct alternate_type<char16_t>
    {
        using result_type = xlang_char8;
    };
}
