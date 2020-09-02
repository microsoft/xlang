#pragma once

#include <RoMetadataApi.h>
#include <hstring.h>
#include <atlbase.h>
#include <windows.foundation.h>
#include <Windows.h>
#include "wil/result.h"
#include "wil/resource.h"
#include "wil/filesystem.h"
#include "Synchapi.h"

#define METADATA_FILE_EXTENSION L"winmd"
#define METADATA_FILE_PATH_FORMAT L"%s%s."  METADATA_FILE_EXTENSION
#define METADATA_FILE_SEARCH_FORMAT L"%s%s*."  METADATA_FILE_EXTENSION

namespace UndockedRegFreeWinRT
{
    // This number comes from (c_cchTypeNameMax = 512) in //depot/fbl_ie_dev1/com/WinRT/buildtools/mdmerge/MetadataScope.h
    static const UINT32 g_uiMaxTypeName = 512;

    typedef enum
    {
        TRO_NONE = 0x00,
        TRO_RESOLVE_TYPE = 0x01,
        TRO_RESOLVE_NAMESPACE = 0x02,
        TRO_RESOLVE_TYPE_AND_NAMESPACE = TRO_RESOLVE_TYPE | TRO_RESOLVE_NAMESPACE
    } TYPE_RESOLUTION_OPTIONS;

    static wil::unique_process_heap_string g_cachedProcessExeDir;

    // Returned string is cached globally, and should not be freed by the caller.
    HRESULT GetProcessExeDir(PCWSTR* path);

    HRESULT FindTypeInMetaDataFile(
        __in IMetaDataDispenserEx* pMetaDataDispenser,
        __in PCWSTR pszFullName,
        __in PCWSTR pszCandidateFilePath,
        __in TYPE_RESOLUTION_OPTIONS resolutionOptions,
        __deref_opt_out_opt IMetaDataImport2** ppMetaDataImport,
        __out_opt mdTypeDef* pmdTypeDef);

    HRESULT FindTypeInDirectory(
        __in IMetaDataDispenserEx* pMetaDataDispenser,
        __in PCWSTR pszFullName,
        __in PCWSTR pszDirectoryPath,
        __out_opt HSTRING* phstrMetaDataFilePath,
        __deref_opt_out_opt IMetaDataImport2** ppMetaDataImport,
        __out_opt mdTypeDef* pmdTypeDef);

    HRESULT FindTypeInDirectoryWithNormalization(
        __in IMetaDataDispenserEx* pMetaDataDispenser,
        __in PCWSTR pszFullName,
        __in PCWSTR pszDirectoryPath,
        __out_opt HSTRING* phstrMetaDataFilePath,
        __deref_opt_out_opt IMetaDataImport2** ppMetaDataImport,
        __out_opt mdTypeDef* pmdTypeDef);

    HRESULT ResolveThirdPartyType(
        __in IMetaDataDispenserEx* pMetaDataDispenser,
        __in PCWSTR pszFullName,
        __out_opt HSTRING* phstrMetaDataFilePath,
        __deref_opt_out_opt IMetaDataImport2** ppMetaDataImport,
        __out_opt mdTypeDef* pmdTypeDef);
}