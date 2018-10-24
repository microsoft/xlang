#pragma once

#include <stdint.h>

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

template <typename char_type>
struct buffer_info
{
    char_type const* buffer;
    uint32_t length;
};
