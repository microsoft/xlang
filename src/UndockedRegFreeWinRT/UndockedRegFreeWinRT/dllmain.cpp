#include <Windows.h>
#include <synchapi.h>
#include <roapi.h>
#include <windows.foundation.h>
#include <activationregistration.h>
#include <combaseapi.h>
#include <wrl.h>
#include <ctxtcall.h>
#include <Processthreadsapi.h>
#include "../detours/detours.h"
#include "catalog.h"
#include <activation.h>
#include <hstring.h>
#include <VersionHelpers.h>
#include "extwinrt.h"

#define WIN1019H1_BLDNUM 18362

// Ensure that metadata resolution functions are imported so they can be detoured
extern "C"
{
    __declspec(dllimport) HRESULT WINAPI RoGetMetaDataFile(
        const HSTRING name,
        IMetaDataDispenserEx* metaDataDispenser,
        HSTRING* metaDataFilePath,
        IMetaDataImport2** metaDataImport,
        mdTypeDef* typeDefToken);

    __declspec(dllimport) HRESULT WINAPI RoParseTypeName(
        HSTRING typeName,
        DWORD* partsCount,
        HSTRING** typeNameParts);

    __declspec(dllimport) HRESULT WINAPI RoResolveNamespace(
        const HSTRING name,
        const HSTRING windowsMetaDataDir,
        const DWORD packageGraphDirsCount,
        const HSTRING* packageGraphDirs,
        DWORD* metaDataFilePathsCount,
        HSTRING** metaDataFilePaths,
        DWORD* subNamespacesCount,
        HSTRING** subNamespaces);

    __declspec(dllimport) HRESULT WINAPI RoIsApiContractPresent(
        PCWSTR name,
        UINT16 majorVersion,
        UINT16 minorVersion,
        BOOL* present);

    __declspec(dllimport) HRESULT WINAPI RoIsApiContractMajorVersionPresent(
        PCWSTR name,
        UINT16 majorVersion,
        BOOL* present);
}

static decltype(RoActivateInstance)* TrueRoActivateInstance = RoActivateInstance;
static decltype(RoGetActivationFactory)* TrueRoGetActivationFactory = RoGetActivationFactory;
static decltype(RoGetMetaDataFile)* TrueRoGetMetaDataFile = RoGetMetaDataFile;
static decltype(RoResolveNamespace)* TrueRoResolveNamespace = RoResolveNamespace;

std::wstring exeFilePath;

enum class ActivationLocation
{
    CurrentApartment,
    CrossApartmentMTA
};

VERSIONHELPERAPI IsWindowsVersionOrGreaterEx(WORD wMajorVersion, WORD wMinorVersion, WORD wServicePackMajor, WORD wBuildNumber)
{
    OSVERSIONINFOEXW osvi = { sizeof(osvi) };
    DWORDLONG const dwlConditionMask =
        VerSetConditionMask(
            VerSetConditionMask(
                VerSetConditionMask(
                    VerSetConditionMask(
                        0, VER_MAJORVERSION, VER_GREATER_EQUAL),
                    VER_MINORVERSION, VER_GREATER_EQUAL),
                VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL),
            VER_BUILDNUMBER, VER_GREATER_EQUAL);

    osvi.dwMajorVersion = wMajorVersion;
    osvi.dwMinorVersion = wMinorVersion;
    osvi.wServicePackMajor = wServicePackMajor;
    osvi.dwBuildNumber = wBuildNumber;

    return VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR | VER_BUILDNUMBER, dwlConditionMask) != FALSE;
}

VERSIONHELPERAPI IsWindows1019H1OrGreater()
{
    return IsWindowsVersionOrGreaterEx(HIBYTE(_WIN32_WINNT_WIN10), LOBYTE(_WIN32_WINNT_WIN10), 0, WIN1019H1_BLDNUM);
}

VOID CALLBACK EnsureMTAInitializedCallBack
(
    PTP_CALLBACK_INSTANCE instance,
    PVOID                 parameter,
    PTP_WORK              work
)
{
    Microsoft::WRL::ComPtr<IComThreadingInfo> spThreadingInfo;
    CoGetObjectContext(IID_PPV_ARGS(&spThreadingInfo));
}

/* 
In the context callback call to the MTA apartment, there is a bug that prevents COM 
from automatically initializing MTA remoting. It only allows NTA to be intialized 
outside of the NTA and blocks all others. The workaround for this is to spin up another 
thread that is not been CoInitialize. COM treats this thread as a implicit MTA and 
when we call CoGetObjectContext on it we implicitily initialized the MTA. 
*/
HRESULT EnsureMTAInitialized()
{
    TP_CALLBACK_ENVIRON callBackEnviron;
    InitializeThreadpoolEnvironment(&callBackEnviron);
    PTP_POOL pool = CreateThreadpool(nullptr);
    if (pool == nullptr)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    SetThreadpoolThreadMaximum(pool, 1);
    if (!SetThreadpoolThreadMinimum(pool, 1)) 
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    PTP_CLEANUP_GROUP cleanupgroup = CreateThreadpoolCleanupGroup();
    if (cleanupgroup == nullptr)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    SetThreadpoolCallbackPool(&callBackEnviron, pool);
    SetThreadpoolCallbackCleanupGroup(&callBackEnviron,
        cleanupgroup,
        nullptr);
    PTP_WORK ensureMTAInitializedWork = CreateThreadpoolWork(
        &EnsureMTAInitializedCallBack,
        nullptr,
        &callBackEnviron);
    if (ensureMTAInitializedWork == nullptr)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    SubmitThreadpoolWork(ensureMTAInitializedWork);
    CloseThreadpoolCleanupGroupMembers(cleanupgroup,
        false,
        nullptr);
    return S_OK;
}

HRESULT GetActivationLocation(HSTRING activatableClassId, ActivationLocation &activationLocation)
{
    APTTYPE aptType;
    APTTYPEQUALIFIER aptQualifier;
    RETURN_IF_FAILED(CoGetApartmentType(&aptType, &aptQualifier));

    ABI::Windows::Foundation::ThreadingType threading_model;
    RETURN_IF_FAILED(WinRTGetThreadingModel(activatableClassId, &threading_model)); //REGDB_E_CLASSNOTREG
    switch (threading_model)
    {
    case ABI::Windows::Foundation::ThreadingType_BOTH:
        activationLocation = ActivationLocation::CurrentApartment;
        break;
    case ABI::Windows::Foundation::ThreadingType_STA:
        if (aptType == APTTYPE_MTA)
        {
            return RO_E_UNSUPPORTED_FROM_MTA;
        }
        else
        {
            activationLocation = ActivationLocation::CurrentApartment;
        }
        break;
    case ABI::Windows::Foundation::ThreadingType_MTA:
        if (aptType == APTTYPE_MTA)
        {
            activationLocation = ActivationLocation::CurrentApartment;
        }
        else
        {
            activationLocation = ActivationLocation::CrossApartmentMTA;
        }
        break;
    }
    return S_OK;
}


HRESULT WINAPI RoActivateInstanceDetour(HSTRING activatableClassId, IInspectable** instance)
{
    ActivationLocation location;
    HRESULT hr = GetActivationLocation(activatableClassId, location);
    if (hr == REGDB_E_CLASSNOTREG)
    {
        return TrueRoActivateInstance(activatableClassId, instance);
    }
    RETURN_IF_FAILED(hr);

    // Activate in current apartment
    if (location == ActivationLocation::CurrentApartment)
    {
        IActivationFactory* pFactory;
        RETURN_IF_FAILED(WinRTGetActivationFactory(activatableClassId, __uuidof(IActivationFactory), (void**)&pFactory));
        return pFactory->ActivateInstance(instance);
    }

    // Cross apartment MTA activation
    struct CrossApartmentMTAActData {
        HSTRING activatableClassId;
        IStream *stream;
    };

    CrossApartmentMTAActData cbdata{ activatableClassId };
    CO_MTA_USAGE_COOKIE mtaUsageCookie;
    RETURN_IF_FAILED(CoIncrementMTAUsage(&mtaUsageCookie));
    RETURN_IF_FAILED(EnsureMTAInitialized());
    Microsoft::WRL::ComPtr<IContextCallback> defaultContext;
    ComCallData data;
    data.pUserDefined = &cbdata;
    RETURN_IF_FAILED(CoGetDefaultContext(APTTYPE_MTA, IID_PPV_ARGS(&defaultContext)));
    RETURN_IF_FAILED(defaultContext->ContextCallback(
        [](_In_ ComCallData* pComCallData) -> HRESULT
        {
            CrossApartmentMTAActData* data = reinterpret_cast<CrossApartmentMTAActData*>(pComCallData->pUserDefined);
            Microsoft::WRL::ComPtr<IInspectable> instance;
            IActivationFactory* pFactory;
            RETURN_IF_FAILED(WinRTGetActivationFactory(data->activatableClassId, __uuidof(IActivationFactory), (void**)&pFactory));
            RETURN_IF_FAILED(pFactory->ActivateInstance(&instance));
            RETURN_IF_FAILED(CoMarshalInterThreadInterfaceInStream(IID_IInspectable, instance.Get(), &data->stream));
            return S_OK;
        },
        &data, IID_ICallbackWithNoReentrancyToApplicationSTA, 5, nullptr)); // 5 is meaningless.
    RETURN_IF_FAILED(CoGetInterfaceAndReleaseStream(cbdata.stream, IID_IInspectable, (LPVOID*)instance));
    return S_OK;
}

HRESULT WINAPI RoGetActivationFactoryDetour(HSTRING activatableClassId, REFIID iid, void** factory)
{
    ActivationLocation location;
    HRESULT hr = GetActivationLocation(activatableClassId, location);
    if (hr == REGDB_E_CLASSNOTREG)
    {
        return TrueRoGetActivationFactory(activatableClassId, iid, factory);
    }
    RETURN_IF_FAILED(hr);

    // Activate in current apartment
    if (location == ActivationLocation::CurrentApartment)
    {
        IActivationFactory* pFactory;
        RETURN_IF_FAILED(WinRTGetActivationFactory(activatableClassId, iid, factory));
        return S_OK;
    }
    // Cross apartment MTA activation
    struct CrossApartmentMTAActData {
        HSTRING activatableClassId;
        IStream* stream;
    };
    CrossApartmentMTAActData cbdata{ activatableClassId };
    CO_MTA_USAGE_COOKIE mtaUsageCookie;
    RETURN_IF_FAILED(CoIncrementMTAUsage(&mtaUsageCookie));
    RETURN_IF_FAILED(EnsureMTAInitialized());
    Microsoft::WRL::ComPtr<IContextCallback> defaultContext;
    ComCallData data;
    data.pUserDefined = &cbdata;
    RETURN_IF_FAILED(CoGetDefaultContext(APTTYPE_MTA, IID_PPV_ARGS(&defaultContext)));
    defaultContext->ContextCallback(
        [](_In_ ComCallData* pComCallData) -> HRESULT
        {
            CrossApartmentMTAActData* data = reinterpret_cast<CrossApartmentMTAActData*>(pComCallData->pUserDefined);
            Microsoft::WRL::ComPtr<IActivationFactory> pFactory;
            RETURN_IF_FAILED(WinRTGetActivationFactory(data->activatableClassId, __uuidof(IActivationFactory), (void**)&pFactory));
            RETURN_IF_FAILED(CoMarshalInterThreadInterfaceInStream(IID_IActivationFactory, pFactory.Get(), &data->stream));
            return S_OK;
        },
        &data, IID_ICallbackWithNoReentrancyToApplicationSTA, 5, nullptr); // 5 is meaningless.
    RETURN_IF_FAILED(CoGetInterfaceAndReleaseStream(cbdata.stream, IID_IActivationFactory, factory));
    return S_OK;
}

HRESULT WINAPI RoGetMetaDataFileDetour(
    const HSTRING name,
    IMetaDataDispenserEx* metaDataDispenser,
    HSTRING* metaDataFilePath,
    IMetaDataImport2** metaDataImport,
    mdTypeDef* typeDefToken)
{
    HRESULT hr = WinRTGetMetadataFile(name, metaDataDispenser, metaDataFilePath, metaDataImport, typeDefToken);
    if (FAILED(hr))
    {
        hr = TrueRoGetMetaDataFile(name, metaDataDispenser, metaDataFilePath, metaDataImport, typeDefToken);
    }
    return hr;
}

HRESULT WINAPI RoResolveNamespaceDetour(
    const HSTRING name,
    const HSTRING windowsMetaDataDir,
    const DWORD packageGraphDirsCount,
    const HSTRING* packageGraphDirs,
    DWORD* metaDataFilePathsCount,
    HSTRING** metaDataFilePaths,
    DWORD* subNamespacesCount,
    HSTRING** subNamespaces)
{
    HRESULT hr = TrueRoResolveNamespace(name, Microsoft::WRL::Wrappers::HStringReference(exeFilePath.c_str()).Get(),
        packageGraphDirsCount, packageGraphDirs,
        metaDataFilePathsCount, metaDataFilePaths,
        subNamespacesCount, subNamespaces);

    if (FAILED(hr))
    {
        hr = TrueRoResolveNamespace(name, windowsMetaDataDir,
            packageGraphDirsCount, packageGraphDirs,
            metaDataFilePathsCount, metaDataFilePaths,
            subNamespacesCount, subNamespaces);
    }
    return hr;
}

void InstallHooks()
{
    if (DetourIsHelperProcess()) {
        return;
    }

    WCHAR filePath[MAX_PATH];
    GetModuleFileNameW(nullptr, filePath, _countof(filePath));
    std::wstring::size_type pos = std::wstring(filePath).find_last_of(L"\\/");
    exeFilePath = std::wstring(filePath).substr(0, pos);

    DetourRestoreAfterWith();

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(PVOID&)TrueRoActivateInstance, RoActivateInstanceDetour);
    DetourAttach(&(PVOID&)TrueRoGetActivationFactory, RoGetActivationFactoryDetour);
    DetourAttach(&(PVOID&)TrueRoGetMetaDataFile, RoGetMetaDataFileDetour);
    DetourAttach(&(PVOID&)TrueRoResolveNamespace, RoResolveNamespaceDetour);
    DetourTransactionCommit();
}

void RemoveHooks()
{

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourDetach(&(PVOID&)TrueRoActivateInstance, RoActivateInstanceDetour);
    DetourDetach(&(PVOID&)TrueRoGetActivationFactory, RoGetActivationFactoryDetour);
    DetourDetach(&(PVOID&)TrueRoGetMetaDataFile, RoGetMetaDataFileDetour);
    DetourDetach(&(PVOID&)TrueRoResolveNamespace, RoResolveNamespaceDetour);
    DetourTransactionCommit();
}

HRESULT ExtRoLoadCatalog()
{
    WCHAR filePath[MAX_PATH];
    GetModuleFileNameW(nullptr, filePath, _countof(filePath));
    std::wstring manifestPath(filePath);
    manifestPath += L".manifest";

    return WinRTLoadComponent(manifestPath.c_str());
}


BOOL WINAPI DllMain(HINSTANCE hmodule, DWORD reason, LPVOID /*lpvReserved*/)
{
    if (IsWindows1019H1OrGreater())
    {
        return true;
    }
    if (reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hmodule);
        InstallHooks();
        ExtRoLoadCatalog();
    }
    if (reason == DLL_PROCESS_DETACH)
    {
        RemoveHooks();
    }
    return true;
}

HRESULT WINAPI RegFreeWinRTInitializeForTest()
{
    InstallHooks();
    ExtRoLoadCatalog();
    return S_OK;
}

HRESULT WINAPI RegFreeWinRTUninitializeForTest()
{
    RemoveHooks();
    return S_OK;
}

extern "C" void WINAPI winrtact_Initialize()
{
    return;
}
