#include "pal_internal.h"
#include "pal_error.h"

#if !XLANG_PLATFORM_WINDOWS
#error "This file is only for targeting Windows"
#endif

#include <Windows.h>
#include <winerror.h>

namespace xlang::impl
{
    inline xlang_result xlang_result_from_hresult(HRESULT error)
    {
        switch (error)
        {
        case S_OK:
            return xlang_result::xlang_success;
        case E_ACCESSDENIED:
            return xlang_result::xlang_access_denied;
        case E_BOUNDS:
            return xlang_result::xlang_bounds;
        case E_FAIL:
            return xlang_result::xlang_fail;
        case E_HANDLE:
            return xlang_result::xlang_handle;
        case E_INVALIDARG:
            return xlang_result::xlang_invalid_arg;
        case E_ILLEGAL_STATE_CHANGE:
            return xlang_result::xlang_invalid_state;
        case E_NOINTERFACE:
            return xlang_result::xlang_no_interface;
        case E_NOTIMPL:
            return xlang_result::xlang_not_impl;
        case E_OUTOFMEMORY:
            return xlang_result::xlang_out_of_memory;
        case E_POINTER:
            return xlang_result::xlang_pointer;
        case static_cast<HRESULT>(0x80040154) :
        case static_cast<HRESULT>(0x80131522):
            return xlang_result::xlang_type_load;
        }

        return xlang_result::xlang_fail;
    }

    [[noreturn]] inline void throw_last_error()
    {
        throw_result(xlang_result_from_hresult(HRESULT_FROM_WIN32(::GetLastError())));
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