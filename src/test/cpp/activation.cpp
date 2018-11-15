#include "pch.h"

using namespace winrt;

extern "C"
{
    int32_t __stdcall OS_RoGetActivationFactory(void* classId, guid const& iid, void** factory) noexcept;
}

#ifdef _M_IX86
#pragma comment(linker, "/alternatename:_OS_RoGetActivationFactory@12=_RoGetActivationFactory@12")
#else
#pragma comment(linker, "/alternatename:OS_RoGetActivationFactory=RoGetActivationFactory")
#endif

int32_t WINRT_CALL WINRT_RoGetActivationFactory(void* classId, guid const& iid, void** factory) noexcept
{
    com_ptr<Windows::Foundation::IActivationFactory> result;
    hresult hr = WINRT_GetActivationFactory(classId, result.put_void());

    if (hr)
    {
        return OS_RoGetActivationFactory(classId, iid, factory);
    }

    return result.as(iid, factory);
}
