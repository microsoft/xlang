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
#include <fstream>
#include <unordered_map>
#include <codecvt>
#include <locale>
#include <RoMetadataApi.h>
#include "catalog.h"
#include "TypeResolution.h"

using namespace std;
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;

// TODO: Components won't respect COM lifetime. workaround to get them in the COM list?

extern "C"
{
    typedef HRESULT(__stdcall* activation_factory_type)(HSTRING, IActivationFactory**);
}

// Intentionally no class factory cache here. That would be excessive since 
// other layers already cache.
struct component
{
    wstring module_name;
    wstring xmlns;
    HMODULE handle = nullptr;
    activation_factory_type get_activation_factory;
    ABI::Windows::Foundation::ThreadingType threading_model;

    ~component()
    {
        if (handle)
        {
            FreeLibrary(handle);
        }
    }

    HRESULT LoadModule()
    {
        if (handle == nullptr)
        {
            handle = LoadLibraryExW(module_name.c_str(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
            if (handle == nullptr)
            {
                return HRESULT_FROM_WIN32(GetLastError());
            }
            this->get_activation_factory = (activation_factory_type)GetProcAddress(handle, "DllGetActivationFactory");
            if (this->get_activation_factory == nullptr)
            {
                return HRESULT_FROM_WIN32(GetLastError());
            }
        }
        return (handle != nullptr && this->get_activation_factory != nullptr) ? S_OK : E_FAIL;
    }

    HRESULT GetActivationFactory(HSTRING className, REFIID  iid, void** factory)
    {
        RETURN_IF_FAILED(LoadModule());

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

static unordered_map<wstring, shared_ptr<component>> g_types;

HRESULT LoadFromSxSManifest(std::wstring path)
{
    return WinRTLoadComponentFromFilePath(path.c_str());
}

HRESULT LoadFromEmbeddedManifest(std::wstring path)
{
    int resource = 0;
    if (path.size() < 4)
    {
        return COR_E_ARGUMENT;
    }
    std::wstring ext = path.substr(path.size() - 4, path.size());
    if (ext.compare(L".exe") == 0)
    {
        resource = 1;
    }
    else if (ext.compare(L".dll") == 0)
    {
        // I notice in testing, the original sxs allows this number to be also 1. 
        resource = 2;
    }
    else
    {
        return COR_E_ARGUMENT;
    }

    HMODULE handle = LoadLibraryExW(path.c_str(), nullptr, LOAD_LIBRARY_AS_DATAFILE_EXCLUSIVE);
    if (!handle)
    {
        return ERROR_FILE_NOT_FOUND;
    }

    HRSRC hrsc = FindResourceW(handle, MAKEINTRESOURCEW(resource), RT_MANIFEST);
    if (!hrsc)
    {
        return ERROR_FILE_NOT_FOUND;
    }
    HGLOBAL embeddedManifest = LoadResource(handle, hrsc);
    if (!embeddedManifest)
    {
        return ERROR_FILE_NOT_FOUND;
    }
    DWORD length = SizeofResource(handle, hrsc);
    void* data = LockResource(embeddedManifest);
    if (!data)
    {
        return ERROR_FILE_NOT_FOUND;
    }
    std::string result = std::string((char*)data, length);
    return WinRTLoadComponentFromString(result.c_str());
}

HRESULT WinRTLoadComponentFromFilePath(PCWSTR manifestPath)
{
    ComPtr<IStream> fileStream;
    RETURN_IF_FAILED(SHCreateStreamOnFileEx(manifestPath, STGM_READ, FILE_ATTRIBUTE_NORMAL, false, nullptr, &fileStream));
    return ParseXmlReaderManfestInput(fileStream.Get());;
}

HRESULT WinRTLoadComponentFromString(PCSTR xmlStringValue)
{
    ComPtr<IStream> xmlStream = nullptr;
    xmlStream.Attach(SHCreateMemStream(reinterpret_cast<const BYTE*>(xmlStringValue), strlen(xmlStringValue) * sizeof(CHAR)));
    RETURN_HR_IF_NULL(E_OUTOFMEMORY, xmlStream);
    ComPtr<IXmlReaderInput> xmlReaderInput;
    RETURN_IF_FAILED(CreateXmlReaderInputWithEncodingName(xmlStream.Get(), nullptr, L"utf-8", FALSE, nullptr, &xmlReaderInput));\
    return ParseXmlReaderManfestInput(xmlReaderInput.Get());
}

HRESULT ParseXmlReaderManfestInput(IUnknown* input)
{
    XmlNodeType nodeType;
    LPCWSTR localName = nullptr;
    auto locale = _create_locale(LC_ALL, "C");
    ComPtr<IXmlReader> xmlReader;
    RETURN_IF_FAILED(CreateXmlReader(__uuidof(IXmlReader), (void**)&xmlReader, nullptr));
    RETURN_IF_FAILED(xmlReader->SetInput(input));
    while (S_OK == xmlReader->Read(&nodeType))
    {
        if (nodeType == XmlNodeType_Element)
        {
            RETURN_IF_FAILED((xmlReader->GetLocalName(&localName, nullptr)));

            if (_wcsicmp_l(localName, L"file", locale) == 0)
            {
                RETURN_IF_FAILED(ParseFileTag(xmlReader));
            }
          
            if (_wcsicmp_l(localName, L"dependentAssembly", locale) == 0)
            {
                RETURN_IF_FAILED(ParseDependentAssemblyTag(xmlReader));
            }
        }
    }

    return S_OK;
}

HRESULT ParseFileTag(ComPtr<IXmlReader> xmlReader)
{
    HRESULT hr = S_OK;
    XmlNodeType nodeType;
    LPCWSTR localName = nullptr;
    LPCWSTR fileName = nullptr;
    hr = xmlReader->MoveToAttributeByName(L"name", nullptr);
    if (hr != S_OK)
    {
        return HRESULT_FROM_WIN32(ERROR_SXS_MANIFEST_PARSE_ERROR);
    }
    RETURN_IF_FAILED(xmlReader->GetValue(&fileName, nullptr));
    if (fileName == nullptr || !fileName[0])
    {
        return HRESULT_FROM_WIN32(ERROR_SXS_MANIFEST_PARSE_ERROR);
    }
    auto locale = _create_locale(LC_ALL, "C");
    while (S_OK == xmlReader->Read(&nodeType))
    {
        if (nodeType == XmlNodeType_Element)
        {
            RETURN_IF_FAILED((xmlReader->GetLocalName(&localName, nullptr)));
            if (_wcsicmp_l(localName, L"activatableClass", locale) == 0)
            {
                RETURN_IF_FAILED(ParseActivatableClassTag(xmlReader, fileName));
            }
        }
        else if (nodeType == XmlNodeType_EndElement)
        {
            RETURN_IF_FAILED((xmlReader->GetLocalName(&localName, nullptr)));
            if (_wcsicmp_l(localName, L"file", locale) == 0)
            {
                return S_OK;
            }
        }
    }
    return S_OK;
}

HRESULT ParseActivatableClassTag(ComPtr<IXmlReader> xmlReader, LPCWSTR fileName)
{
    LPCWSTR localName = nullptr;
    auto locale = _create_locale(LC_ALL, "C");
    auto this_component = make_shared<component>();
    this_component->module_name = fileName;
    HRESULT hr = xmlReader->MoveToFirstAttribute();
    // Using this pattern intead of MoveToAttributeByName improves performance
    const WCHAR* activatableClass = nullptr;
    const WCHAR* threadingModel = nullptr;
    const WCHAR* xmlns = nullptr;
    if (S_FALSE == hr)
    {
        return HRESULT_FROM_WIN32(ERROR_SXS_MANIFEST_PARSE_ERROR);
    }
    else
    {
        while (TRUE)
        {
            const WCHAR* pwszLocalName;
            const WCHAR* pwszValue;
            if (FAILED(hr = xmlReader->GetLocalName(&pwszLocalName, NULL)))
            {
                wprintf(L"Error getting local name, error is %08.8lx", hr);
                return HRESULT_FROM_WIN32(ERROR_SXS_MANIFEST_PARSE_ERROR);
            }
            if (FAILED(hr = xmlReader->GetValue(&pwszValue, NULL)))
            {
                wprintf(L"Error getting value, error is %08.8lx", hr);
                return HRESULT_FROM_WIN32(ERROR_SXS_MANIFEST_PARSE_ERROR);
            }
            if (_wcsicmp_l(L"threadingModel", pwszLocalName, locale) == 0)
            {
                threadingModel = pwszValue;
            }
            else if (_wcsicmp_l(L"name", pwszLocalName, locale) == 0)
            {
                activatableClass = pwszValue;
            }
            else if (_wcsicmp_l(L"xmlns", pwszLocalName, locale) == 0)
            {
                xmlns = pwszValue;
            }

            if (xmlReader->MoveToNextAttribute() != S_OK)
            {
                break;
            }
        }
    }
    if (threadingModel == nullptr)
    {
        return HRESULT_FROM_WIN32(ERROR_SXS_MANIFEST_PARSE_ERROR);
    }
    if (_wcsicmp_l(L"sta", threadingModel, locale) == 0)
    {
        this_component->threading_model = ABI::Windows::Foundation::ThreadingType::ThreadingType_STA;
    }
    else if (_wcsicmp_l(L"mta", threadingModel, locale) == 0)
    {
        this_component->threading_model = ABI::Windows::Foundation::ThreadingType::ThreadingType_MTA;
    }
    else if (_wcsicmp_l(L"both", threadingModel, locale) == 0)
    {
        this_component->threading_model = ABI::Windows::Foundation::ThreadingType::ThreadingType_BOTH;
    }
    else
    {
        return HRESULT_FROM_WIN32(ERROR_SXS_MANIFEST_PARSE_ERROR);
    }

    if (activatableClass == nullptr || !activatableClass[0])
    {
        return HRESULT_FROM_WIN32(ERROR_SXS_MANIFEST_PARSE_ERROR);
    }
    this_component->xmlns = xmlns; // Should we care if this value is blank or missing?
    // Check for duplicate activatable classes
    auto component_iter = g_types.find(activatableClass);
    if (component_iter != g_types.end()) 
    {
        return HRESULT_FROM_WIN32(ERROR_SXS_DUPLICATE_ACTIVATABLE_CLASS);
    }
    g_types[activatableClass] = this_component;
    return S_OK;
}

HRESULT ParseDependentAssemblyTag(ComPtr<IXmlReader> xmlReader)
{
    XmlNodeType nodeType;
    LPCWSTR localName = nullptr;
    auto locale = _create_locale(LC_ALL, "C");
    while (S_OK == xmlReader->Read(&nodeType))
    {
        if (nodeType == XmlNodeType_Element)
        {
            RETURN_IF_FAILED((xmlReader->GetLocalName(&localName, nullptr)));
            if (_wcsicmp_l(localName, L"assemblyIdentity", locale) == 0)
            {
                RETURN_IF_FAILED(ParseAssemblyIdentityTag(xmlReader));
            }
        }
        else if (nodeType == XmlNodeType_EndElement)
        {
            RETURN_IF_FAILED((xmlReader->GetLocalName(&localName, nullptr)));
            if (_wcsicmp_l(localName, L"dependentAssembly", locale) == 0)
            {
                return S_OK;
            }
        }
    }
    return S_OK;
}

HRESULT ParseAssemblyIdentityTag(ComPtr<IXmlReader> xmlReader)
{
    HRESULT hr = S_OK;
    LPCWSTR dependentAssemblyFileName = nullptr;
    hr = xmlReader->MoveToAttributeByName(L"name", nullptr);
    if (hr != S_OK)
    {
        return HRESULT_FROM_WIN32(ERROR_SXS_MANIFEST_PARSE_ERROR);
    }
    RETURN_IF_FAILED(xmlReader->GetValue(&dependentAssemblyFileName, nullptr));
    if (dependentAssemblyFileName == nullptr || !dependentAssemblyFileName[0])
    {
        return HRESULT_FROM_WIN32(ERROR_SXS_MANIFEST_PARSE_ERROR);
    }
    PCWSTR exeFilePath = nullptr;
    UndockedRegFreeWinRT::GetProcessExeDir(&exeFilePath);
    std::wstring path = std::wstring(exeFilePath) + L"\\";
    std::wstring dllPath = path + std::wstring(dependentAssemblyFileName) + L".dll";
    hr = LoadFromEmbeddedManifest(dllPath);
    if (hr == ERROR_FILE_NOT_FOUND)
    {
        std::wstring sxsManifestPath = path + std::wstring(dependentAssemblyFileName) + L".manifest";
        return LoadFromSxSManifest(sxsManifestPath.c_str());
    }
    return hr;
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
    mdTypeDef* typeDefToken)
{
    wchar_t folderPrefix[9];
    PCWSTR pszFullName = WindowsGetStringRawBuffer(name, nullptr);
    HRESULT hr = StringCchCopyW(folderPrefix, _countof(folderPrefix), pszFullName);
    if (hr != S_OK && hr != STRSAFE_E_INSUFFICIENT_BUFFER)
    {
        return hr;
    }
    if (CompareStringOrdinal(folderPrefix, -1, L"Windows.", -1, false) == CSTR_EQUAL)
    {
        return RO_E_METADATA_NAME_NOT_FOUND;
    }

    if (metaDataFilePath != nullptr)
    {
        *metaDataFilePath = nullptr;
    }
    if (metaDataImport != nullptr)
    {
        *metaDataImport = nullptr;
    }
    if (typeDefToken != nullptr)
    {
        *typeDefToken = mdTypeDefNil;
    }

    if (((metaDataImport == nullptr) && (typeDefToken != nullptr)) ||
        ((metaDataImport != nullptr) && (typeDefToken == nullptr)))
    {
        return E_INVALIDARG;
    }

    ComPtr<IMetaDataDispenserEx> spMetaDataDispenser;
    // The API uses the caller's passed-in metadata dispenser. If null, it
    // will create an instance of the metadata reader to dispense metadata files.
    if (metaDataDispenser == nullptr)
    {
        RETURN_IF_FAILED(CoCreateInstance(CLSID_CorMetaDataDispenser,
            nullptr,
            CLSCTX_INPROC,
            IID_IMetaDataDispenser,
            &spMetaDataDispenser));
        {
            variant_t version{ L"WindowsRuntime 1.4" };
            RETURN_IF_FAILED(spMetaDataDispenser->SetOption(MetaDataRuntimeVersion, &version.GetVARIANT()));
        }
    }
    return UndockedRegFreeWinRT::ResolveThirdPartyType(
        spMetaDataDispenser.Get(),
        pszFullName,
        metaDataFilePath,
        metaDataImport,
        typeDefToken);
}