#include "pal_internal.h"
#include "pal_error.h"

#if !XLANG_PLATFORM_WINDOWS
#error "This file is only for targeting Windows"
#endif

#include <Windows.h>
#include <winerror.h>

namespace xlang::impl
{
    [[noreturn]] inline void throw_last_error()
    {
        throw_result(HRESULT_FROM_WIN32(::GetLastError()));
    }

    template<typename T>
    void check_bool(T result)
    {
        if (!result)
        {
            throw_last_error();
        }
    }
}