#include <stdlib.h>
#include "pal.h"

#ifdef _WIN32
#error "This file is for targeting platforms other than Windows"
#endif

extern "C"
{
    void* XLANG_CALL XlangMemAlloc(size_t count)
    {
        if (count == 0)
        {
            count = 1;
        }
        return ::malloc(count);
    }

    void XLANG_CALL XlangMemFree(void* ptr)
    {
        ::free(ptr);
    }
}
