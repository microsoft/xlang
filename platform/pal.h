#ifndef XLANG_PAL_H
#define XLANG_PAL_H

#include <stddef.h>
#include <stdint.h>

#if defined(_MSC_VER) && defined(_M_IX86)
#define XLANG_CALL __stdcall
#elif defined(__clang__) && defined(__i386__)
#define XLANG_CALL __stdcall
#else
#define XLANG_CALL
#endif

#ifndef XLANG_PAL_EXPORT
# ifdef _MSC_VER
#  ifdef XLANG_PAL_EXPORTS
#   define XLANG_PAL_EXPORT __declspec(dllexport)
#  else
#   define XLANG_PAL_EXPORT __declspec(dllimport)
#  endif
# endif
#endif

extern "C"
{
    XLANG_PAL_EXPORT void* XLANG_CALL XlangMemAlloc(size_t count);
    XLANG_PAL_EXPORT void XLANG_CALL XlangMemFree(void* ptr);
}

#endif