
#include <Windows.h>
#include <synchapi.h>
#include <roapi.h>
#include <rometadataresolution.h>
#include <windows.foundation.h>
#include <detours.h>
#include <catalog.h>
#include <extwinrt.h>
#include <activationregistration.h>

static decltype(RoActivateInstance)* TrueRoActivateInstance = RoActivateInstance;
static decltype(RoGetActivationFactory)* TrueRoGetActivationFactory = RoGetActivationFactory;
static decltype(RoGetMetaDataFile)* TrueRoGetMetaDataFile = RoGetMetaDataFile;
static decltype(RoResolveNamespace)* TrueRoResolveNamespace = RoResolveNamespace;

HRESULT WINAPI RoActivateInstanceDetour(HSTRING activatableClassId, IInspectable** instance
)
{
    IActivationFactory* pFactory;
	HRESULT hr;

	APTTYPE aptType;
	APTTYPEQUALIFIER aptQualifier;
	CoGetApartmentType(&aptType, &aptQualifier);

	DWORD threading_model;
	hr = WinRTGetThreadingModel(activatableClassId, &threading_model);
	
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
	if (activationLocation == Here)
	{
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
	{
		// Cross apartment MTA activation
	}
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

