#include <Windows.h>
#include <synchapi.h>
#include <roapi.h>
#include <rometadataresolution.h>
#include <windows.foundation.h>
#include <activationregistration.h>
#include <combaseapi.h>
#include <wrl.h>
#include <ctxtcall.h>
#include <Processthreadsapi.h>

#include "../detours/detours.h"
#include "catalog.h"
#include "extwinrt.h"

static decltype(RoActivateInstance)* TrueRoActivateInstance = RoActivateInstance;
static decltype(RoGetActivationFactory)* TrueRoGetActivationFactory = RoGetActivationFactory;
static decltype(RoGetMetaDataFile)* TrueRoGetMetaDataFile = RoGetMetaDataFile;
static decltype(RoResolveNamespace)* TrueRoResolveNamespace = RoResolveNamespace;

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

HRESULT WINAPI RoActivateInstanceDetour(HSTRING activatableClassId, IInspectable** instance)
{
    APTTYPE aptType;
    APTTYPEQUALIFIER aptQualifier;
    RETURN_IF_FAILED(CoGetApartmentType(&aptType, &aptQualifier));
    enum
    {
        CurrentApartment,
        CrossApartmentMTA
    } activationLocation = CurrentApartment;

    ABI::Windows::Foundation::ThreadingType threading_model;
    if (WinRTGetThreadingModel(activatableClassId, &threading_model) == REGDB_E_CLASSNOTREG)
    {
        return E_FAIL;
    }
    switch (threading_model)
    {
    case ABI::Windows::Foundation::ThreadingType_BOTH:
        activationLocation = CurrentApartment;
        break;
    case ABI::Windows::Foundation::ThreadingType_STA:
        if (aptType == APTTYPE_MTA)
        {
            return RO_E_UNSUPPORTED_FROM_MTA;
        }
        else
        {
            activationLocation = CurrentApartment;
        }
        break;
    case ABI::Windows::Foundation::ThreadingType_MTA:
        if (aptType == APTTYPE_MTA)
        {
            activationLocation = CurrentApartment;
        }
        else
        {
            activationLocation = CrossApartmentMTA;
        }
        break;
    }
    // Activate in current apartment
    if (activationLocation == CurrentApartment)
    {
        IActivationFactory* pFactory;
        RETURN_IF_FAILED(WinRTGetActivationFactory(activatableClassId, __uuidof(IActivationFactory), (void**)&pFactory));
        return pFactory->ActivateInstance(instance);
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
    RETURN_IF_FAILED(CoGetApartmentType(&aptType, &aptQualifier));
    Microsoft::WRL::ComPtr<IContextCallback> defaultContext;
    ComCallData data;
    data.pUserDefined = &cbdata;
    RETURN_IF_FAILED(CoGetDefaultContext(APTTYPE_MTA, IID_PPV_ARGS(&defaultContext)));
    RETURN_IF_FAILED(defaultContext->ContextCallback(
        [](_In_ ComCallData* pComCallData) -> HRESULT
        {
            CrossApartmentMTAActData* data = reinterpret_cast<CrossApartmentMTAActData*>(pComCallData->pUserDefined);
            Microsoft::WRL::ComPtr<IInspectable> instance;
            HRESULT hr;
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
    HRESULT hr = WinRTGetActivationFactory(activatableClassId, iid, factory);
    if (hr == REGDB_E_CLASSNOTREG)
    {
        hr = TrueRoGetActivationFactory(activatableClassId, iid, factory);
    }
    return hr;
}

HRESULT WINAPI RoGetMetaDataFileDetour(
    const HSTRING name,
    IMetaDataDispenserEx* metaDataDispenser,
    HSTRING* metaDataFilePath,
    IMetaDataImport2** metaDataImport,
    mdTypeDef* typeDefToken)
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
    const DWORD packageGraphDirsCount,
    const HSTRING* packageGraphDirs,
    DWORD* metaDataFilePathsCount,
    HSTRING** metaDataFilePaths,
    DWORD* subNamespacesCount,
    HSTRING** subNamespaces)
{
    HRESULT hr = WinRTResolveNamespaceDetour(name, windowsMetaDataDir,
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

void InstallHooks()
{
    if (DetourIsHelperProcess()) {
        return;
    }

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

HRESULT WINAPI ExtRoLoadCatalog(PCWSTR componentPath)
{
    return WinRTLoadComponent(componentPath);
}

BOOL WINAPI DllMain(HINSTANCE /*hmodule*/, DWORD reason, LPVOID /*lpvReserved*/)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        InstallHooks();
    }
    if (reason == DLL_PROCESS_DETACH)
    {
        RemoveHooks();
    }
    return true;
}

