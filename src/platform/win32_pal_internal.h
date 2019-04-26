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
            return xlang_result::success;
        case E_ACCESSDENIED:
            return xlang_result::access_denied;
        case E_BOUNDS:
            return xlang_result::bounds;
        case E_FAIL:
            return xlang_result::fail;
        case E_HANDLE:
            return xlang_result::handle;
        case E_INVALIDARG:
            return xlang_result::invalid_arg;
        case E_ILLEGAL_STATE_CHANGE:
            return xlang_result::invalid_state;
        case E_NOINTERFACE:
            return xlang_result::no_interface;
        case E_NOTIMPL:
            return xlang_result::not_impl;
        case E_OUTOFMEMORY:
            return xlang_result::out_of_memory;
        case E_POINTER:
            return xlang_result::pointer;
        case static_cast<HRESULT>(0x80040154):
        case static_cast<HRESULT>(0x80131522):
            return xlang_result::type_load;
        }

        return xlang_result::fail;
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