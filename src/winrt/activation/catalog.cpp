#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING 

#include <Windows.h>
#include <roapi.h>
#include <winstring.h>
#include <rometadataresolution.h>

#include <iostream>
#include <fstream>
#include <unordered_map>
#include <codecvt>
#include <locale>
#include <string>

#include <catalog.h>

using namespace std;

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
    wstring metadata_path;
    HMODULE handle = nullptr;
    activation_factory_type get_activation_factory;

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
    auto this_component = make_shared<component>();
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

    // first line is the name of the DLL that contains the types
    ifstream manifest_file(manifest_path, ios_base::in);
    std::string u8_module_name;
    getline(manifest_file, u8_module_name);
    this_component->module_path = converter.from_bytes(u8_module_name);
    printf("Registering module: %s\n", u8_module_name.c_str());

    // second line is the name of the winmd that contains the types (in case of CLR, can be the same as dll)
    string u8_metadata_path;
    getline(manifest_file, u8_metadata_path);
    this_component->metadata_path = converter.from_bytes(u8_metadata_path);
    printf("Registering metadata: %s\n", u8_metadata_path.c_str());

    // each line after is an activatable class. No apartment-specific registration. Implicitly "both".
    string u8_class_name;
    while (getline(manifest_file, u8_class_name))
    {
        std::wstring wide_class_name = converter.from_bytes(u8_class_name);
        g_types[wide_class_name] = this_component;
        printf("Registering class: %s\n", u8_class_name.c_str());
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
    return REGDB_E_CLASSNOTREG;
}
