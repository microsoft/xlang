// Temporary win32 placeholder

#include <Windows.h>
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
        static_assert(sizeof(char16_t) == sizeof(wchar_t));
        static_assert(sizeof(xlang_char8) == sizeof(char));
        wchar_t const* input = reinterpret_cast<wchar_t const*>(input_str);
        char* output = reinterpret_cast<char*>(output_buffer);

        int result = ::WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, input, input_length, output, buffer_length, nullptr, nullptr);
        if (result > 0)
        {
            return static_cast<uint32_t>(result);
        }
        else
        {
            switch (GetLastError())
            {
            case ERROR_INSUFFICIENT_BUFFER:
                throw_result(xlang_error_mem_invalid_size);

            case ERROR_NO_UNICODE_TRANSLATION:
                throw_result(xlang_error_untranslatable_string);

            default:
                throw_result(xlang_error_sadness);
            }
        }
    }
    
    uint32_t convert_string(
        xlang_char8 const* input_str,
        uint32_t input_length,
        char16_t* output_buffer,
        uint32_t buffer_length)
    {
        static_assert(sizeof(char16_t) == sizeof(wchar_t));
        static_assert(sizeof(xlang_char8) == sizeof(char));
        char const* input = reinterpret_cast<char const*>(input_str);
        wchar_t* output = reinterpret_cast<wchar_t*>(output_buffer);

        int result = ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, input, input_length, output, buffer_length);
        if (result > 0)
        {
            return static_cast<uint32_t>(result);
        }
        else
        {
            switch (GetLastError())
            {
            case ERROR_INSUFFICIENT_BUFFER:
                throw_result(xlang_error_mem_invalid_size);

            case ERROR_NO_UNICODE_TRANSLATION:
                throw_result(xlang_error_untranslatable_string);

            default:
                throw_result(xlang_error_sadness);
            }
        }
    }
}