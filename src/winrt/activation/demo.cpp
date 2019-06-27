
#include <Windows.h>
#include <roapi.h>
#include <rometadataresolution.h>
#include <windows.foundation.h>
#include <stdio.h>
#include <detours.h>
#include <wrl\client.h>
#include <wrl\wrappers\corewrappers.h>

#include <catalog.h>

using namespace Microsoft::WRL;
using namespace ABI::Windows::Foundation;
using namespace Microsoft::WRL::Wrappers;

// Target pointer for the uninstrumented Sleep API.
static decltype(RoActivateInstance)* TrueRoActivateInstance = RoActivateInstance;
static decltype(RoGetActivationFactory)* TrueRoGetActivationFactory = RoGetActivationFactory;
static decltype(RoGetMetaDataFile)* TrueRoGetMetaDataFile = RoGetMetaDataFile;
static decltype(Sleep)* TrueSleep = Sleep;
static decltype(RoResolveNamespace)* TrueRoResolveNamespace = RoResolveNamespace;

// Detour function that replaces the Sleep API.
VOID WINAPI TimedSleep(DWORD dwMilliseconds)
{
    // Save the before and after times around calling the Sleep API.
    printf("zzz...\n");
    TrueSleep(dwMilliseconds);
}

HRESULT WINAPI RoActivateInstanceDetour(
    HSTRING      activatableClassId,
    IInspectable** instance
)
{
    IActivationFactory* pFactory;
    HRESULT hr = WinRTGetActivationFactory(activatableClassId, __uuidof(IActivationFactory), (void**)&pFactory);
    if (hr == REGDB_E_CLASSNOTREG)
    {
        hr =  TrueRoActivateInstance(activatableClassId, instance);
    }
    else if (SUCCEEDED(hr))
    {
        hr = pFactory->ActivateInstance(instance);
    }
    return hr;
}

HRESULT WINAPI RoGetActivationFactoryDetour(
    HSTRING activatableClassId,
    REFIID  iid,
    void** factory
)
{
    HRESULT hr = WinRTGetActivationFactory(activatableClassId, iid, factory);
    if (hr == REGDB_E_CLASSNOTREG)
    {
        hr = TrueRoGetActivationFactory(activatableClassId, iid, factory);
    }
    return hr;
}

HRESULT WINAPI RoGetMetaDataFileDetour(
    const HSTRING        name,
    IMetaDataDispenserEx* metaDataDispenser,
    HSTRING* metaDataFilePath,
    IMetaDataImport2** metaDataImport,
    mdTypeDef* typeDefToken
)
{
    HRESULT hr = WinRTGetMetadataFile(name, metaDataDispenser, metaDataFilePath, metaDataImport, typeDefToken);
    if (hr == REGDB_E_CLASSNOTREG)
    {
        hr = TrueRoGetMetaDataFile(name, metaDataDispenser, metaDataFilePath, metaDataImport, typeDefToken);
    }
    return hr;
}

HRESULT WINAPI RoResolveNamespaceDetour(
    const HSTRING name,
    const HSTRING windowsMetaDataDir,
    const DWORD   packageGraphDirsCount,
    const HSTRING* packageGraphDirs,
    DWORD* metaDataFilePathsCount,
    HSTRING** metaDataFilePaths,
    DWORD* subNamespacesCount,
    HSTRING** subNamespaces)

{
    HRESULT hr = TrueRoResolveNamespace(name, windowsMetaDataDir,
        packageGraphDirsCount, packageGraphDirs,
        metaDataFilePathsCount, metaDataFilePaths,
        subNamespacesCount, subNamespaces);
    if (hr == REGDB_E_CLASSNOTREG)
    {
        hr = TrueRoResolveNamespace(name, windowsMetaDataDir,
            packageGraphDirsCount, packageGraphDirs,
            metaDataFilePathsCount, metaDataFilePaths,
            subNamespacesCount, subNamespaces);
    }
    return hr;
}

int main(int argc, char *argv[])
{
    if (DetourIsHelperProcess()) {
        return TRUE;
    }

    DetourRestoreAfterWith();

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(PVOID&)TrueSleep, TimedSleep);
    DetourAttach(&(PVOID&)TrueRoActivateInstance, RoActivateInstanceDetour);
    DetourAttach(&(PVOID&)TrueRoGetActivationFactory, RoGetActivationFactoryDetour);
    DetourAttach(&(PVOID&)TrueRoGetMetaDataFile, RoGetMetaDataFileDetour);
    DetourTransactionCommit();

    // basic sniff-test
    Sleep(1000);

    WinRTLoadComponent(L"demo.txt");
    WinRTLoadComponent(L"demo2.txt");

    HRESULT hr = S_OK;

    {
        ComPtr<IInspectable> instance;
        hr = RoActivateInstance(HStringReference(L"test_component.Class").Get(), &instance);
        if (FAILED(hr)) { printf("Failed activate: %x\n", hr); return false; }

        HString result;
        instance->GetRuntimeClassName(result.GetAddressOf());
        if (FAILED(hr)) { printf("Failed to load name: %x\n", hr); return false; }

        unsigned int length = 0;
        wprintf(L"%s\n", result.GetRawBuffer(&length));
    }

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

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourDetach(&(PVOID&)TrueSleep, TimedSleep);
    DetourDetach(&(PVOID&)TrueRoActivateInstance, RoActivateInstanceDetour);
    DetourDetach(&(PVOID&)TrueRoGetActivationFactory, RoGetActivationFactoryDetour);
    DetourDetach(&(PVOID&)TrueRoGetMetaDataFile, RoGetMetaDataFileDetour);
    DetourTransactionCommit();

    return TRUE;
}
