#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING 

#include <Windows.h>
#include <roapi.h>
#include <winstring.h>
#include <rometadataresolution.h>
#include <combaseapi.h>
#include <wrl.h>
#include <xmllite.h>
#include <Shlwapi.h>
#include <comutil.h>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <codecvt>
#include <locale>
#include <string>

#include <catalog.h>

using namespace std;
using namespace Microsoft::WRL;

// TODO: Components won't respect COM lifetime. workaround to get them in the COM list?

extern "C"
{
    typedef HRESULT (__stdcall * activation_factory_type)(HSTRING, IActivationFactory**) ;
}

// Intentionally no class factory cache here. That would be excessive since 
// other layers already cache.
struct component
{
    wstring module_path;
    wstring xmlns;
    HMODULE handle = nullptr;
    activation_factory_type get_activation_factory;
    ABI::Windows::Foundation::ThreadingType threading_model;

    HRESULT LoadModule()
    {
        if (handle == nullptr)
        {
            handle = LoadLibraryW(module_path.c_str());
            if (handle == nullptr)
            {
                return HRESULT_FROM_WIN32(GetLastError());;
            }
            this->get_activation_factory = (activation_factory_type)GetProcAddress(handle, "DllGetActivationFactory");
            if (this->get_activation_factory == nullptr)
            {
                return HRESULT_FROM_WIN32(GetLastError());;
            }
        }
        return (handle != nullptr && this->get_activation_factory != nullptr) ? S_OK : E_FAIL;
    }

    HRESULT GetActivationFactory(HSTRING className, REFIID  iid, void** factory)
    {
        if (FAILED(LoadModule())) return REGDB_E_CLASSNOTREG;

        IActivationFactory* ifactory = nullptr;
        HRESULT hr = this->get_activation_factory(className, &ifactory);
        // optimize for IActivationFactory?
        if (SUCCEEDED(hr))
        {
            hr = ifactory->QueryInterface(iid, factory);
            ifactory->Release();
        }
        return hr;
    }
};

static unordered_map < wstring, shared_ptr<component> > g_types;

HRESULT WinRTLoadComponent(PCWSTR manifest_path)
{
    ComPtr<IStream> fileStream;
    ComPtr<IXmlReader> xmlReader;
    XmlNodeType nodeType;
    const WCHAR* localName = L"";
    auto locale = _create_locale(LC_ALL, "C");
    auto this_component = make_shared<component>();

    RETURN_IF_FAILED(SHCreateStreamOnFileEx(manifest_path, STGM_READ, FILE_ATTRIBUTE_NORMAL, false, nullptr, &fileStream));
    RETURN_IF_FAILED(CreateXmlReader(__uuidof(IXmlReader), (void**)&xmlReader, nullptr));
    RETURN_IF_FAILED(xmlReader->SetInput(fileStream.Get()));
    const WCHAR* fileName;
    while (S_OK == xmlReader->Read(&nodeType))
    {
        if (nodeType == XmlNodeType_Element)
        {
            RETURN_IF_FAILED((xmlReader->GetLocalName(&localName, nullptr)));
            if (_wcsicmp_l(localName, L"file", locale) == 0)
            {
                RETURN_IF_FAILED(xmlReader->MoveToAttributeByName(L"name", nullptr));
                RETURN_IF_FAILED(xmlReader->GetValue(&fileName, nullptr));
                this_component->module_path = fileName;
            }
            else if (_wcsicmp_l(localName, L"activatableClass", locale) == 0)
            {
                const WCHAR* threadingModel;
                RETURN_IF_FAILED(xmlReader->MoveToAttributeByName(L"threadingModel", nullptr));
                RETURN_IF_FAILED(xmlReader->GetValue(&threadingModel, nullptr));

                if (_wcsicmp_l(L"sta", threadingModel, locale) == 0)
                {
                    this_component->threading_model = ABI::Windows::Foundation::ThreadingType::ThreadingType_STA;
                }
                else if (_wcsicmp_l(L"mta", threadingModel, locale) == 0)
                {
                    this_component->threading_model = ABI::Windows::Foundation::ThreadingType::ThreadingType_MTA;
                }
                else if(_wcsicmp_l(L"both", threadingModel, locale) == 0)
                {
                    this_component->threading_model = ABI::Windows::Foundation::ThreadingType::ThreadingType_BOTH;
                }
                else
                {
                    return HRESULT_FROM_WIN32(ERROR_SXS_MANIFEST_PARSE_ERROR);
                }

                const WCHAR* xmlns;
                RETURN_IF_FAILED(xmlReader->MoveToAttributeByName(L"xmlns", nullptr));
                RETURN_IF_FAILED(xmlReader->GetValue(&xmlns, nullptr));
                this_component->xmlns = xmlns;

                const WCHAR* activatableClass;
                RETURN_IF_FAILED(xmlReader->MoveToAttributeByName(L"clsid", nullptr));
                RETURN_IF_FAILED(xmlReader->GetValue(&activatableClass, nullptr));
                g_types[activatableClass] = this_component;
            }
        }
    }

    return S_OK;
}

HRESULT WinRTGetThreadingModel(HSTRING activatableClassId, ABI::Windows::Foundation::ThreadingType* threading_model)
{
    auto raw_class_name = WindowsGetStringRawBuffer(activatableClassId, nullptr);
    auto component_iter = g_types.find(raw_class_name);
    if (component_iter != g_types.end())
    {
        *threading_model = component_iter->second->threading_model;
        return S_OK;
    }
    return REGDB_E_CLASSNOTREG;
}

HRESULT WinRTGetActivationFactory(
    HSTRING activatableClassId,
    REFIID  iid,
    void** factory)
{
    auto raw_class_name = WindowsGetStringRawBuffer(activatableClassId, nullptr);
    auto component_iter = g_types.find(raw_class_name);
    if (component_iter != g_types.end())
    {
        return component_iter->second->GetActivationFactory(activatableClassId, iid, factory);
    }
    return REGDB_E_CLASSNOTREG;
    
}

HRESULT WinRTGetMetadataFile(
    const HSTRING        name,
    IMetaDataDispenserEx* metaDataDispenser,
    HSTRING* metaDataFilePath,
    IMetaDataImport2** metaDataImport,
    mdTypeDef* typeDefToken
)
{
    // documentation: 
    //  https://docs.microsoft.com/en-us/uwp/winrt-cref/winmd-files
    //  https://docs.microsoft.com/en-us/windows/win32/api/rometadataresolution/nf-rometadataresolution-rogetmetadatafile
    
    // algorithm: search our metadata, find best match if any.
    // search system metedata. 
    // Take longest string match excluding extension (which should be winmd, but not clear it's required to be).

    // TODO: Once implented, change the detour hook. no need to call the system API twice.

    return REGDB_E_CLASSNOTREG;
}

HRESULT WinRTResolveNamespaceDetour(
    const HSTRING name,
    const HSTRING windowsMetaDataDir,
    const DWORD   packageGraphDirsCount,
    const HSTRING* packageGraphDirs,
    DWORD* metaDataFilePathsCount,
    HSTRING** metaDataFilePaths,
    DWORD* subNamespacesCount,
    HSTRING** subNamespaces)

{
    // documentation:
    // https://docs.microsoft.com/en-us/windows/win32/api/rometadataresolution/nf-rometadataresolution-roresolvenamespace

    return REGDB_E_CLASSNOTREG;
}
