#include "pal_internal.h"
#include "simple_unicode_converter.h"
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

    namespace converter=xlang::impl::code_converter;

    static size_t if_ok(converter::converter_result r, size_t sz)
    {
        switch (r)
        {
            case converter::converter_result::OK:
                return sz;
            case converter::converter_result::INVALID_INPUT_DATA:
                throw_result(xlang_error_untranslatable_string);
            case converter::converter_result::OUTPUT_TOO_SMALL:
                throw_result(xlang_error_mem_invalid_size);
        }
        throw_result(xlang_error_sadness);
    }
    uint32_t convert_string(std::basic_string_view<char16_t> input_str,
                            xlang_char8 *output_buffer, uint32_t buffer_size)
    {
        static_assert(sizeof(xlang_char8) == sizeof(char));
        bool count_only = (output_buffer == nullptr);
        size_t result_size;
        converter::converter_result status;
        if (count_only)
        {
            status=converter::output_size(
                            input_str.begin(),
                            input_str.end(),
                            converter::utf16_filter{},
                            converter::utf8_filter{},
                            result_size);
        }
        else
        {
        status=converter::convert(
                        input_str.begin(),
                        input_str.end(),
                        output_buffer,
                        output_buffer + buffer_size,
                        converter::utf16_filter{},
                        converter::utf8_filter{},
                        result_size);
        }
        return static_cast<uint32_t>(if_ok(status,result_size));
    }

    uint32_t convert_string(std::basic_string_view<xlang_char8> input_str,
                            char16_t *output_buffer, uint32_t buffer_size)
    {
        static_assert(sizeof(xlang_char8) == sizeof(char));
        bool count_only = (output_buffer == nullptr);
        converter::converter_result status;
        size_t result_size;
        if (count_only)
        {
            status=converter::output_size(
                            input_str.begin(),
                            input_str.end(),
                            converter::utf8_filter{},
                            converter::utf16_filter{},
                            result_size);
        }
        else
        {
            status=converter::convert(
                        input_str.begin(),
                        input_str.end(),
                        output_buffer,
                        output_buffer + buffer_size,
                        converter::utf8_filter{},
                        converter::utf16_filter{},
                        result_size);
        }
        return static_cast<uint32_t>(if_ok(status,result_size));
    }
}
