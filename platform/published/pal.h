#pragma once

#ifndef XLANG_PAL_H
#define XLANG_PAL_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
#include <type_traits>
#endif

#ifdef _WIN32
#define XLANG_PLATFORM_WINDOWS 1
#else
#define XLANG_PLATFORM_WINDOWS 0
#endif

#ifdef _MSC_VER
#define XLANG_COMPILER_CLANG 0
#define XLANG_COMPILER_MSVC 1
#elif
#define XLANG_COMPILER_CLANG 1
#define XLANG_COMPILER_MSVC 0
#else
#error Xlang only tested against Visual C++ and clang
#endif


#if XLANG_COMPILER_MSVC && defined(_M_IX86)
#define XLANG_CALL __stdcall
#elif defined(__clang__) && defined(__i386__)
#define XLANG_CALL __stdcall
#else
#define XLANG_CALL
#endif

#ifndef XLANG_PAL_EXPORT
# if XLANG_PLATFORM_WINDOWS
#  ifdef XLANG_PAL_EXPORTS
#   define XLANG_PAL_EXPORT __declspec(dllexport)
#  else
#   define XLANG_PAL_EXPORT __declspec(dllimport)
#  endif
# else
#  ifdef XLANG_PAL_EXPORTS
#    define XLANG_PAL_EXPORT __attribute__ ((visibility ("default")))
#  else
#    define XLANG_PAL_EXPORT 
#  endif
# endif
#endif

#ifdef __cplusplus
#define XLANG_NOEXCEPT noexcept
#else
#define XLANG_NOEXCEPT
#endif

#ifndef XLANG_PAL_HAS_CHAR8_T
# ifdef __cpp_char8_t
#  define XLANG_PAL_HAS_CHAR8_T 1
# else
#  define XLANG_PAL_HAS_CHAR8_T 0
# endif
#endif

#if XLANG_PAL_HAS_CHAR8_T
#define XLANG_PAL_CHAR8_T char8_t
#else
#define XLANG_PAL_CHAR8_T char
#endif

#ifdef __cplusplus
extern "C"
{
#endif
    // Type/handle definitions
    typedef uint32_t xlang_result;

    typedef XLANG_PAL_CHAR8_T xlang_char8;

    struct xlang_string__
    {
        int unused;
    };
    typedef xlang_string__* xlang_string;

    struct xlang_string_buffer__
    {
        int unused;
    };
    typedef xlang_string_buffer__* xlang_string_buffer;

    struct xlang_string_header
    {
        void* reserved1;
        char reserved2[16];
    };

#ifdef __cplusplus
    enum class xlang_string_encoding
    {
        none = 0x0,
        utf8 = 0x1,
        utf16 = 0x2
    };

    inline constexpr xlang_string_encoding operator|(xlang_string_encoding lhs, xlang_string_encoding rhs) noexcept
    {
        using int_t = std::underlying_type_t<xlang_string_encoding>;
        return static_cast<xlang_string_encoding>(static_cast<int_t>(lhs) | static_cast<int_t>(rhs));
    }

    inline constexpr xlang_string_encoding operator&(xlang_string_encoding lhs, xlang_string_encoding rhs) noexcept
    {
        using int_t = std::underlying_type_t<xlang_string_encoding>;
        return static_cast<xlang_string_encoding>(static_cast<int_t>(lhs) & static_cast<int_t>(rhs));
    }

    inline constexpr xlang_string_encoding& operator|=(xlang_string_encoding& lhs, xlang_string_encoding rhs) noexcept
    {
        lhs = lhs | rhs;
        return lhs;
    }

    inline constexpr xlang_string_encoding& operator&=(xlang_string_encoding& lhs, xlang_string_encoding rhs) noexcept
    {
        lhs = lhs & rhs;
        return lhs;
    }
#else
    enum xlang_string_encoding
    {
        XlangStringEncodingNone = 0x0,
        XlangStringEncodingUtf8 = 0x1,
        XlangStringEncodingUtf16 = 0x2
    };
#endif

    // Function declarations
    XLANG_PAL_EXPORT void* XLANG_CALL xlang_mem_alloc(size_t count) XLANG_NOEXCEPT;

    XLANG_PAL_EXPORT void XLANG_CALL xlang_mem_free(void* ptr) XLANG_NOEXCEPT;

    XLANG_PAL_EXPORT xlang_result XLANG_CALL xlang_create_string_utf8(
        xlang_char8 const* source_string,
        uint32_t length,
        xlang_string* string
    ) XLANG_NOEXCEPT;
    XLANG_PAL_EXPORT xlang_result XLANG_CALL xlang_create_string_utf16(
        char16_t const* source_string,
        uint32_t length,
        xlang_string* string
    ) XLANG_NOEXCEPT;

    XLANG_PAL_EXPORT xlang_result XLANG_CALL xlang_create_string_reference_utf8(
        xlang_char8 const* source_string,
        uint32_t length,
        xlang_string_header* header,
        xlang_string* string
    ) XLANG_NOEXCEPT;
    XLANG_PAL_EXPORT xlang_result XLANG_CALL xlang_create_string_reference_utf16(
        char16_t const* source_string,
        uint32_t length,
        xlang_string_header* header,
        xlang_string* string
    ) XLANG_NOEXCEPT;

    XLANG_PAL_EXPORT void XLANG_CALL xlang_delete_string(xlang_string string) XLANG_NOEXCEPT;

    XLANG_PAL_EXPORT xlang_result XLANG_CALL xlang_delete_string_buffer(xlang_string_buffer buffer_handle) XLANG_NOEXCEPT;

    XLANG_PAL_EXPORT xlang_result XLANG_CALL xlang_duplicate_string(
        xlang_string string,
        xlang_string* newString
    ) XLANG_NOEXCEPT;

    XLANG_PAL_EXPORT xlang_string_encoding XLANG_CALL xlang_get_string_encoding(
        xlang_string string
    ) XLANG_NOEXCEPT;

    XLANG_PAL_EXPORT xlang_result XLANG_CALL xlang_get_string_raw_buffer_utf8(
        xlang_string string,
        xlang_char8 const* * buffer,
        uint32_t* length
    ) XLANG_NOEXCEPT;
    XLANG_PAL_EXPORT xlang_result XLANG_CALL xlang_get_string_raw_buffer_utf16(
        xlang_string string,
        char16_t const* * buffer,
        uint32_t* length
    ) XLANG_NOEXCEPT;

    XLANG_PAL_EXPORT xlang_result XLANG_CALL xlang_preallocate_string_buffer_utf8(
        uint32_t length,
        xlang_char8** char_buffer,
        xlang_string_buffer* buffer_handle
    ) XLANG_NOEXCEPT;
    XLANG_PAL_EXPORT xlang_result XLANG_CALL xlang_preallocate_string_buffer_utf16(
        uint32_t length,
        char16_t** char_buffer,
        xlang_string_buffer* buffer_handle
    ) XLANG_NOEXCEPT;

    XLANG_PAL_EXPORT xlang_result XLANG_CALL xlang_promote_string_buffer(
        xlang_string_buffer buffer_handle,
        xlang_string* string,
        uint32_t length
    ) XLANG_NOEXCEPT;

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
inline constexpr xlang_result xlang_error_ok{ 0 };
inline constexpr xlang_result xlang_error_mem_invalid_size{ 0x80080011 };
inline constexpr xlang_result xlang_error_string_not_null_terminated{ 0x80000017 };
inline constexpr xlang_result xlang_error_pointer{ 0x80004003 };
inline constexpr xlang_result xlang_error_sadness{ 0x80004005 };
inline constexpr xlang_result xlang_error_out_of_memory{ 0x8007000e };
inline constexpr xlang_result xlang_error_invalid_arg{ 0x80070057 };
inline constexpr xlang_result xlang_error_untranslatable_string{ 0x80070459 };
#endif

#endif