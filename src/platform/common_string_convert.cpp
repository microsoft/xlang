#include "pal_internal.h"
#include "string_convert.h"

namespace xlang::impl
{
    uint32_t get_converted_length(std::basic_string_view<char16_t> input_str)
    {
        return convert_string(input_str, nullptr, 0);
    }

    uint32_t get_converted_length(std::basic_string_view<xlang_char8> input_str)
    {
        return convert_string(input_str, nullptr, 0);
    }

    uint32_t convert_string(
        std::basic_string_view<char16_t> input_str,
        xlang_char8* output_buffer,
        uint32_t buffer_size)
    {
        static_assert(sizeof(xlang_char8) == sizeof(char));
 
        // TODO: implement convert_string for non-windows platforms (#47)
        throw_result(xlang_error_sadness);
    }
    
    uint32_t convert_string(
        std::basic_string_view<xlang_char8> input_str,
        char16_t* output_buffer,
        uint32_t buffer_size)
    {
        static_assert(sizeof(xlang_char8) == sizeof(char));

        // TODO: implement convert_string for non-windows platforms (#47)
        throw_result(xlang_error_sadness);
    }
}