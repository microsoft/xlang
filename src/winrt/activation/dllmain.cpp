
#include <Windows.h>
#include <synchapi.h>
#include <roapi.h>
#include <rometadataresolution.h>
#include <windows.foundation.h>
#include <detours.h>
#include <catalog.h>
#include <extwinrt.h>
#include <activationregistration.h>
#include <combaseapi.h>
#include <wrl.h>
#include <ctxtcall.h>
#include <Processthreadsapi.h>

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

void EnsureMTAInitialized()
{
	TP_CALLBACK_ENVIRON callBackEnviron;
	InitializeThreadpoolEnvironment(&callBackEnviron);
	PTP_POOL pool = CreateThreadpool(nullptr);
	SetThreadpoolThreadMaximum(pool, 1);
	SetThreadpoolThreadMinimum(pool, 1);
	PTP_CLEANUP_GROUP cleanupgroup = CreateThreadpoolCleanupGroup();
	HRESULT hr;
	SetThreadpoolCallbackPool(&callBackEnviron, pool);
	SetThreadpoolCallbackCleanupGroup(&callBackEnviron,
		cleanupgroup,
		nullptr);
	PTP_WORK ensureMTAInitializedWork = CreateThreadpoolWork(&EnsureMTAInitializedCallBack,
		nullptr,
		&callBackEnviron);
	SubmitThreadpoolWork(ensureMTAInitializedWork);
	CloseThreadpoolCleanupGroupMembers(cleanupgroup,
		false,
		nullptr);
}

HRESULT WINAPI RoActivateInstanceDetour(HSTRING activatableClassId, IInspectable** instance
)
{
	APTTYPE aptType;
	APTTYPEQUALIFIER aptQualifier;
	CoGetApartmentType(&aptType, &aptQualifier);

	DWORD threading_model;
	HRESULT hr = WinRTGetThreadingModel(activatableClassId, &threading_model);
	
	enum
	{
		Here,
		CrossApartmentMTA
	} activationLocation = Here;

	if (hr == REGDB_E_CLASSNOTREG)
	{
	    hr =  TrueRoActivateInstance(activatableClassId, instance);
	}
	switch (threading_model)
	{
		case static_cast<DWORD>(ABI::Windows::Foundation::ThreadingType_BOTH) :
			activationLocation = Here;
			break;
		case static_cast<DWORD>(ABI::Windows::Foundation::ThreadingType_STA) :
			if (aptType == APTTYPE_MTA)
			{
				return RO_E_UNSUPPORTED_FROM_MTA;
			}
			else
			{
				activationLocation = Here;
			}
		break;
		case static_cast<DWORD>(ABI::Windows::Foundation::ThreadingType_MTA) :
			if (aptType == APTTYPE_MTA)
			{
				activationLocation = Here;
			}
			else
			{
				activationLocation = CrossApartmentMTA;
			}
		break;
	}
	/*if (activationLocation == Here)
	{
		IActivationFactory* pFactory;
		hr = WinRTGetActivationFactory(activatableClassId, __uuidof(IActivationFactory), (void**)&pFactory);
		if (hr == REGDB_E_CLASSNOTREG)
		{
			hr = TrueRoActivateInstance(activatableClassId, instance);
		}
		if (SUCCEEDED(hr))
		{
			hr = pFactory->ActivateInstance(instance);
		}
	}
	else
	{*/
		struct CBData {
			HSTRING hs;
			IStream *strm;
		};
		CBData cbdata{ activatableClassId };
		CO_MTA_USAGE_COOKIE mtaUsageCookie;
		hr = CoIncrementMTAUsage(&mtaUsageCookie);
		EnsureMTAInitialized();
		if (FAILED(hr))
		{
			return hr;
		}
		CoGetApartmentType(&aptType, &aptQualifier);
		Microsoft::WRL::ComPtr<IContextCallback> defaultContext;
		ComCallData data;
		data.pUserDefined = &cbdata;
		hr = CoGetDefaultContext(APTTYPE_MTA, IID_PPV_ARGS(&defaultContext));
		if (SUCCEEDED(hr))
		{
			hr = defaultContext->ContextCallback(
				[](_In_ ComCallData* pComCallData) -> HRESULT
				{
					CBData* data = reinterpret_cast<CBData*>(pComCallData->pUserDefined);
					Microsoft::WRL::ComPtr<IInspectable> instance;
					HRESULT hr;
					IActivationFactory* pFactory;
					hr = WinRTGetActivationFactory(data->hs, __uuidof(IActivationFactory), (void**)&pFactory);
					if (hr == REGDB_E_CLASSNOTREG)
					{
						hr = TrueRoActivateInstance(data->hs, &instance);
					}
					if (SUCCEEDED(hr))
					{
						hr = pFactory->ActivateInstance(&instance);
					}
					if (SUCCEEDED(hr))
					{
						return CoMarshalInterThreadInterfaceInStream(IID_IInspectable, instance.Get(), &data->strm);
					}
					return hr;
				}, 
				&data, IID_ICallbackWithNoReentrancyToApplicationSTA, 5, nullptr); // 5 is meaningless.

			if (SUCCEEDED(hr))
			{
				hr = CoGetInterfaceAndReleaseStream(cbdata.strm, IID_IInspectable, (LPVOID*)instance);
			}
		}
		// Cross apartment MTA activation
	//}
    return hr;
}

HRESULT WINAPI RoGetActivationFactoryDetour(HSTRING activatableClassId, REFIID iid, void** factory
)
{
    HRESULT hr = WinRTGetActivationFactory(activatableClassId, iid, factory);
    /*if (hr == REGDB_E_CLASSNOTREG)
    {
        hr = TrueRoGetActivationFactory(activatableClassId, iid, factory);
    }*/
    return hr;
}

HRESULT WINAPI RoGetMetaDataFileDetour(
    const HSTRING name,
    IMetaDataDispenserEx* metaDataDispenser,
    HSTRING* metaDataFilePath,
    IMetaDataImport2** metaDataImport,
    mdTypeDef* typeDefToken
)
{
    HRESULT hr = WinRTGetMetadataFile(name, metaDataDispenser, metaDataFilePath, metaDataImport, typeDefToken);
    /*if (hr == REGDB_E_CLASSNOTREG)
    {
        hr = TrueRoGetMetaDataFile(name, metaDataDispenser, metaDataFilePath, metaDataImport, typeDefToken);
    }*/
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
    DetourTransactionCommit();
}

void RemoveHooks()
{

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourDetach(&(PVOID&)TrueRoActivateInstance, RoActivateInstanceDetour);
    DetourDetach(&(PVOID&)TrueRoGetActivationFactory, RoGetActivationFactoryDetour);
    DetourDetach(&(PVOID&)TrueRoGetMetaDataFile, RoGetMetaDataFileDetour);
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

