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

    class fallback_xlang_traits
    {
      public:
        [[noreturn]] static void data_error()
        {
            throw_result(xlang_error_untranslatable_string);
        }
        [[noreturn]] static void buffer_error()
        {
            throw_result(xlang_error_mem_invalid_size);
        }
    };

    // create a fallback converter that knows how to throw xlang errors.
    using converter = simple_unicode_converter<fallback_xlang_traits>;

    uint32_t convert_string(std::basic_string_view<char16_t> input_str,
                            xlang_char8 *output_buffer, uint32_t buffer_size)
    {
        static_assert(sizeof(xlang_char8) == sizeof(char));
        bool count_only = (output_buffer == nullptr);
        return static_cast<uint32_t>(
                    converter::convert(
                        input_str.begin(),
                        input_str.end(),
                        output_buffer,
                        output_buffer + buffer_size,
                        converter::utf16_filter{},
                        converter::utf8_filter{},
                        count_only));
    }

    uint32_t convert_string(std::basic_string_view<xlang_char8> input_str,
                            char16_t *output_buffer, uint32_t buffer_size)
    {
        static_assert(sizeof(xlang_char8) == sizeof(char));
        bool count_only = (output_buffer == nullptr);
        return static_cast<uint32_t>(
                    converter::convert(
                        input_str.begin(),
                        input_str.end(),
                        output_buffer,
                        output_buffer + buffer_size,
                        converter::utf8_filter{},
                        converter::utf16_filter{},
                        count_only));
    }
}
