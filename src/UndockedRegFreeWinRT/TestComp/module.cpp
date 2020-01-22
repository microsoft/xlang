#include <wrl\module.h>
#include <wrl\implements.h>
#include <combaseapi.h>
#include <windows.foundation.h>
#include <activation.h>

STDAPI __stdcall DllCanUnloadNow()
{
    return Microsoft::WRL::Module<Microsoft::WRL::InProc>::GetModule().GetObjectCount() == 0;
}

STDAPI __stdcall DllGetActivationFactory(_In_ HSTRING classId, _COM_Outptr_ IActivationFactory** factory) noexcept
{
    auto& module = Microsoft::WRL::Module<Microsoft::WRL::InProc>::GetModule();
    return module.GetActivationFactory(classId, factory);
}