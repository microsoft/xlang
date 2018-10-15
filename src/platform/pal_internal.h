#pragma once

#include <pal.h>
#include <new>

#ifdef _DEBUG

#define XLANG_ASSERT _ASSERTE
#define XLANG_VERIFY XLANG_ASSERT
#define XLANG_VERIFY_(result, expression) XLANG_ASSERT(result == expression)

#else

#define XLANG_ASSERT(expression) ((void)0)
#define XLANG_VERIFY(expression) (void)(expression)
#define XLANG_VERIFY_(result, expression) (void)(expression)

#endif

namespace xlang
{
    struct xlang_error
    {
        xlang_result result;
    };

    [[noreturn]] inline void throw_result(xlang_result result)
    {
        throw xlang_error{ result };
    }

    inline xlang_result to_result() noexcept
    {
        try
        {
            throw;
        }
        catch (xlang_error const& e)
        {
            return e.result;
        }
        catch (std::bad_alloc const&)
        {
            return xlang_error_out_of_memory;
        }
    }
}