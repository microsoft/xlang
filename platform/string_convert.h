#pragma once

#include "pal.h"
#include <stdint.h>
#include <optional>

namespace xlang::impl
{
    uint32_t get_converted_length(char16_t const* input_str, uint32_t input_length);
    uint32_t get_converted_length(xlang_char8 const* input_str, uint32_t input_length);

    uint32_t convert_string(
        char16_t const* input_str,
        uint32_t input_length,
        xlang_char8* output_buffer,
        uint32_t buffer_size);

    uint32_t convert_string(
        xlang_char8 const* input_str,
        uint32_t input_length,
        char16_t* output_buffer,
        uint32_t buffer_size);
}