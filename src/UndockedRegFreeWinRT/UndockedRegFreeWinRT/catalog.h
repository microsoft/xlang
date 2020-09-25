#include <activationregistration.h>
#include <string>
#include <cor.h>
#include <xmllite.h>
#include <Shlwapi.h>

#include "wil/result.h"
#include "wil/resource.h"

HRESULT LoadFromSxSManifest(std::wstring path);

HRESULT LoadFromEmbeddedManifest(std::wstring path);

HRESULT WinRTLoadComponentFromFilePath(PCWSTR manifestPath);

HRESULT WinRTLoadComponentFromString(PCSTR xmlStringValue);

HRESULT ParseXmlReaderManfestInput(IUnknown* pInput);

HRESULT ParseFileTag(Microsoft::WRL::ComPtr<IXmlReader> xmlReader, LPCWSTR fileName);

HRESULT ParseDependentAssemblyTag(Microsoft::WRL::ComPtr<IXmlReader> xmlReader);

HRESULT ParseActivatableClassTag(Microsoft::WRL::ComPtr<IXmlReader> xmlReader, LPCWSTR fileName);

HRESULT ParseAssemblyIdentityTag(Microsoft::WRL::ComPtr<IXmlReader> xmlReader);

HRESULT WinRTGetThreadingModel(
    HSTRING activatableClassId,
    ABI::Windows::Foundation::ThreadingType* threading_model);

HRESULT WinRTGetActivationFactory(
    HSTRING activatableClassId,
    REFIID  iid,
    void** factory);

HRESULT WinRTGetMetadataFile(
    const HSTRING        name,
    IMetaDataDispenserEx* metaDataDispenser,
    HSTRING* metaDataFilePath,
    IMetaDataImport2** metaDataImport,
    mdTypeDef* typeDefToken);