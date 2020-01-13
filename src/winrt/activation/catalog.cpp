#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING 

#include <Windows.h>
#include <roapi.h>
#include <winstring.h>
#include <rometadataresolution.h>
#include <combaseapi.h>
#include <wrl.h>
#include <msxml6.h>
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
    HMODULE handle = nullptr;
    activation_factory_type get_activation_factory;
	DWORD threading_model;

    bool LoadModule() 
    {
        if (handle == nullptr)
        {
            handle = LoadLibraryW(module_path.c_str());
            this->get_activation_factory = (activation_factory_type)GetProcAddress(handle, "DllGetActivationFactory");
        }
        return handle != nullptr && this->get_activation_factory != nullptr;
    }

    HRESULT GetActivationFactory(HSTRING className, REFIID  iid, void** factory)
    {
        if (!LoadModule()) return REGDB_E_CLASSNOTREG;

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
    VARIANT_BOOL vbResult = VARIANT_FALSE;

    ComPtr<IXMLDOMDocument2> xmlDoc;
    RETURN_IF_FAILED(CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(xmlDoc.GetAddressOf())));
    HRESULT hr = xmlDoc->load(_variant_t(manifest_path), &vbResult);
    if (hr != S_OK)
    {
        // load can return S_FALSE on failure
        return hr;
    }

    ComPtr<IXMLDOMNodeList> fileList;
    xmlDoc->setProperty(_bstr_t(L"SelectionNamespaces"), _variant_t(L"xmlns:m='urn:schemas-microsoft-com:asm.v1'"));
    BSTR query = SysAllocString(L"/m:assembly/m:file");
    RETURN_IF_FAILED(xmlDoc->selectNodes(query, &fileList));
    if (fileList == nullptr)
    {
        return S_OK;
    }

    ComPtr<IXMLDOMNode> file;
    while (SUCCEEDED(fileList->nextNode(&file)) && file)
    {
        ComPtr<IXMLDOMNamedNodeMap> attributes;
        ComPtr<IXMLDOMNode> attribute;
        ComPtr<IXMLDOMNodeList> comClassList;
        ComPtr<IXMLDOMNode> comClass;
        BSTR value;

        auto this_component = make_shared<component>();

        RETURN_IF_FAILED(file->get_attributes(&attributes));
        RETURN_IF_FAILED(attributes->getNamedItem(BSTR(L"name"), &attribute));
        RETURN_IF_FAILED(attribute->get_text(&value));
        this_component->module_path = value;

        RETURN_IF_FAILED(file->get_childNodes(&comClassList));
        while (SUCCEEDED(comClassList->nextNode(&comClass)) && comClass)
        {
            RETURN_IF_FAILED(comClass->get_attributes(&attributes));
            RETURN_IF_FAILED(attributes->getNamedItem(BSTR(L"threadingModel"), &attribute));
            RETURN_IF_FAILED(attribute->get_text(&value));

            if (wcsicmp(L"apartment", value) == 0)
            {
                this_component->threading_model = ABI::Windows::Foundation::ThreadingType::ThreadingType_STA;
            }
            else if (wcsicmp(L"free", value) == 0)
            {
                this_component->threading_model = ABI::Windows::Foundation::ThreadingType::ThreadingType_MTA;
            }
            else
            {
                this_component->threading_model = ABI::Windows::Foundation::ThreadingType::ThreadingType_BOTH;
            }

            RETURN_IF_FAILED(attributes->getNamedItem(BSTR(L"clsid"), &attribute));
            RETURN_IF_FAILED(attribute->get_text(&value));
            g_types[value] = this_component;
        }
    }

    return S_OK;
}

HRESULT WinRTGetActivationFactory(
    HSTRING activatableClassId,
    REFIID  iid,
    void** factory)
{
    uint32_t length;
    auto raw_class_name = WindowsGetStringRawBuffer(activatableClassId, &length);
    auto component_iter = g_types.find(raw_class_name);
    if (component_iter != g_types.end())
    {
        return component_iter->second->GetActivationFactory(activatableClassId, iid, factory);
    }
    else
    {
        return REGDB_E_CLASSNOTREG;
    }
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
