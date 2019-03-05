#include "pch.h"

bool WINRT_CALL test_can_unload_now() noexcept;
void* WINRT_CALL test_get_activation_factory(std::wstring_view const& name);

int32_t WINRT_CALL DllCanUnloadNow() noexcept
{
#ifdef _WRL_MODULE_H_
    if (!::Microsoft::WRL::Module<::Microsoft::WRL::InProc>::GetModule().Terminate())
    {
        return 1;
    }
#endif

    return test_can_unload_now() ? 0 : 1;
}

int32_t WINRT_CALL DllGetActivationFactory(void* classId, void** factory) noexcept
{
    try
    {
        uint32_t length{};
        wchar_t const* const buffer = WINRT_WindowsGetStringRawBuffer(classId, &length);
        std::wstring_view const name{ buffer, length };

        *factory = test_get_activation_factory(name);

        if (*factory)
        {
            return 0;
        }

#ifdef _WRL_MODULE_H_
        return ::Microsoft::WRL::Module<::Microsoft::WRL::InProc>::GetModule().GetActivationFactory(static_cast<HSTRING>(classId), reinterpret_cast<::IActivationFactory * *>(factory));
#else
        return winrt::hresult_class_not_available(name).to_abi();
#endif
    }
    catch (...) { return winrt::to_hresult(); }
}
