#include "winrt/base.h"

extern "C"
{
    int32_t WINRT_CALL WINRT_RoInitialize(uint32_t type) noexcept
    {
        return winrt::impl::error_ok;
    }

    void WINRT_CALL WINRT_RoUninitialize() noexcept
    {
    }

    void* WINRT_CoTaskMemAlloc(size_t cb) noexcept
    {
        return malloc(cb);
    }

    void WINRT_CoTaskMemFree(void* pv) noexcept
    {
        free(pv);
    }
}