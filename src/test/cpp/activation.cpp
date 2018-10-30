#include "pch.h"

using namespace winrt;

int32_t WINRT_CALL WINRT_RoGetActivationFactory(void* classId, guid const& iid, void** factory) noexcept
{
    com_ptr<Windows::Foundation::IActivationFactory> result;
    hresult hr = WINRT_GetActivationFactory(classId, result.put_void());

    if (hr)
    {
        return hr;
    }

    return result.as(iid, factory);
}
