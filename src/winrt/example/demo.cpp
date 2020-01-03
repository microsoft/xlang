
#include <Windows.h>
#include <roapi.h>
#include <rometadataresolution.h>
#include <windows.foundation.h>
#include <stdio.h>
#include <wrl\client.h>
#include <wrl\wrappers\corewrappers.h>
#include <extwinrt.h>

using namespace Microsoft::WRL;
using namespace ABI::Windows::Foundation;
using namespace Microsoft::WRL::Wrappers;

int main(int argc, char *argv[])
{
    ExtRoLoadCatalog(L"demo.txt");
    ExtRoLoadCatalog(L"demo2.txt");

    HRESULT hr = S_OK;
	CoInitialize(nullptr);
    //{
    //    ComPtr<IInspectable> instance;
    //    hr = RoActivateInstance(HStringReference(L"test_component.Class").Get(), &instance);
    //    if (FAILED(hr)) { printf("Failed activate: %x\n", hr); return false; }

    //    HString result;
    //    instance->GetRuntimeClassName(result.GetAddressOf());
    //    if (FAILED(hr)) { printf("Failed to load name: %x\n", hr); return false; }

    //    unsigned int length = 0;
    //    wprintf(L"%s\n", result.GetRawBuffer(&length));
    //}

    {
        ComPtr<IInspectable> instance;
        hr = RoActivateInstance(HStringReference(L"MyComponent.SampleClass").Get(), &instance);
        if (FAILED(hr)) { printf("Failed activate: %x\n", hr); return false; }

        HString result;
        instance->GetRuntimeClassName(result.GetAddressOf());
        if (FAILED(hr)) { printf("Failed to load name: %x\n", hr); return false; }

        unsigned int length = 0;
        wprintf(L"%s\n", result.GetRawBuffer(&length));
    }
	CoUninitialize();
    return 0;
}
