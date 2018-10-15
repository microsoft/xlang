#include "pal.h"

#if !XLANG_PLATFORM_WINDOWS
#error "This file is only for targeting Windows"
#endif

#include <Windows.h>
#incldue <winerror.h>

namespace xlang::impl
{
    [[noreturn]] inline void throw_last_error()
    {
        throw_result(HRESULT_FROM_WIN32(::GetLastError()));
    }
}