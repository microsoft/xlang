#include "pal_internal.h"
#include "string_convert.h"

namespace xlang::impl
{
    uint32_t get_converted_length(char16_t const* input_str, uint32_t input_length)
    {
        return convert_string(input_str, input_length, nullptr, 0);
    }

    uint32_t get_converted_length(xlang_char8 const* input_str, uint32_t input_length)
    {
        return convert_string(input_str, input_length, nullptr, 0);
    }

    uint32_t convert_string(
        char16_t const* input_str,
        uint32_t input_length,
        xlang_char8* output_buffer,
        uint32_t buffer_length)
    {
        static_assert(sizeof(xlang_char8) == sizeof(char));
 
        // TODO: implement convert_string for non-windows platforms (#47)
        throw_result(xlang_error_sadness);
    }
    
    uint32_t convert_string(
        xlang_char8 const* input_str,
        uint32_t input_length,
        char16_t* output_buffer,
        uint32_t buffer_length)
    {
        static_assert(sizeof(xlang_char8) == sizeof(char));

        // TODO: implement convert_string for non-windows platforms (#47)
        throw_result(xlang_error_sadness);
    }
}