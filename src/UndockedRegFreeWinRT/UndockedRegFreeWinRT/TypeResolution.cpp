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
#include "TypeResolution.h"
#include "catalog.h"
#include <iostream>

namespace UndockedRegFreeWinRT
{
    BOOL CALLBACK GetProcessExeDirInitOnceCallback(
        _Inout_     PINIT_ONCE,
        _Inout_opt_ PVOID,
        _Out_opt_   PVOID*)
    {
        // The raw GetModuleFileName API doesn't have any way of telling the caller how large of a buffer is needed.
        // Historically, this has driven people to use it with MAX_PATH, but MAX_PATH is evil because we want to support
        // long filenames. So instead, call this WIL wrapper, which allocates successively larger buffers until it finds one
        // that's big enough.
        wil::unique_process_heap_string localExePath;
        HRESULT hr = wil::GetModuleFileNameW(nullptr, localExePath);
        if (FAILED_LOG(hr))
        {
            SetLastError(hr);
            return FALSE;
        }

        // Modify the retrieved string to truncate the actual exe name and leave the containing directory path. This API
        // expects a buffer size including the terminating null, so add 1 to the string length.
        hr = PathCchRemoveFileSpec(localExePath.get(), wcslen(localExePath.get()) + 1);
        if (FAILED_LOG(hr))
        {
            SetLastError(hr);
            return FALSE;
        }

        g_cachedProcessExeDir = std::move(localExePath);
        return TRUE;
    }

    // Returned string is cached globally, and should not be freed by the caller.
    HRESULT GetProcessExeDir(PCWSTR* path)
    {
        *path = nullptr;
        static INIT_ONCE ProcessExeDirInitOnce = INIT_ONCE_STATIC_INIT;

        RETURN_IF_WIN32_BOOL_FALSE(InitOnceExecuteOnce(&ProcessExeDirInitOnce, GetProcessExeDirInitOnceCallback, nullptr, nullptr));

        // The cache has been successfully populated by the InitOnce, so we can just use it directly.
        *path = g_cachedProcessExeDir.get();
        return S_OK;
    }

    HRESULT FindTypeInMetaDataFile(
        __in IMetaDataDispenserEx* pMetaDataDispenser,
        __in PCWSTR pszFullName,
        __in PCWSTR pszCandidateFilePath,
        __in TYPE_RESOLUTION_OPTIONS resolutionOptions,
        __deref_opt_out_opt IMetaDataImport2** ppMetaDataImport,
        __out_opt mdTypeDef* pmdTypeDef)
    {
        HRESULT hr = S_OK;
        CComPtr<IMetaDataImport2> spMetaDataImport;
        hr = pMetaDataDispenser->OpenScope(
            pszCandidateFilePath,
            ofReadOnly,
            IID_IMetaDataImport2,
            reinterpret_cast<IUnknown**>(&spMetaDataImport));
        std::wcout << "hr3 " << hr << std::endl;
        if (SUCCEEDED(hr))
        {
            const size_t cFullName = wcslen(pszFullName);
            wchar_t pszRetrievedName[g_uiMaxTypeName];
            HCORENUM hEnum = nullptr;
            mdTypeDef rgTypeDefs[32];
            ULONG cTypeDefs;
            DWORD dwTypeDefProps;
            hr = RO_E_METADATA_NAME_NOT_FOUND;

            if (TRO_RESOLVE_TYPE & resolutionOptions)
            {
                hr = spMetaDataImport->FindTypeDefByName(pszFullName, mdTokenNil, &rgTypeDefs[0]);
                if (SUCCEEDED(hr))
                {
                    //  Check to confirm that the type we just found is a
                    //  winrt type.  If it is, we're good, otherwise we
                    //  want to fail with RO_E_INVALID_METADATA_FILE.
                    hr = spMetaDataImport->GetTypeDefProps(rgTypeDefs[0], nullptr, 0, nullptr, &dwTypeDefProps, nullptr);
                    if (SUCCEEDED(hr))
                    {
                        //  If we found the type but it's not a winrt type,
                        //  it's an error.
                        //
                        //  If the type is public, than the metadata file
                        //  is corrupt (all public types in a winrt file
                        //  must be tdWindowsRuntime).  If the type is
                        //  private, then we just want to report that the
                        //  type wasn't found.
                        if (!IsTdWindowsRuntime(dwTypeDefProps))
                        {
                            if (IsTdPublic(dwTypeDefProps))
                            {
                                hr = RO_E_INVALID_METADATA_FILE;
                            }
                            else
                            {
                                hr = RO_E_METADATA_NAME_NOT_FOUND;
                            }
                        }
                    }
                    else
                    {
                        hr = RO_E_INVALID_METADATA_FILE;
                    }
                    if (SUCCEEDED(hr))
                    {
                        if (pmdTypeDef != nullptr)
                        {
                            *pmdTypeDef = rgTypeDefs[0];
                        }
                        if (ppMetaDataImport != nullptr)
                        {
                            *ppMetaDataImport = spMetaDataImport.Detach();
                        }
                    }
                }
                else if (hr == CLDB_E_RECORD_NOTFOUND)
                {
                    hr = RO_E_METADATA_NAME_NOT_FOUND;
                }
            }

            if ((hr == RO_E_METADATA_NAME_NOT_FOUND) &&
                (TRO_RESOLVE_NAMESPACE & resolutionOptions))
            {
                // Check whether the name is a namespace rather than a type.
                do
                {
                    hr = spMetaDataImport->EnumTypeDefs(
                        &hEnum,
                        rgTypeDefs,
                        ARRAYSIZE(rgTypeDefs),
                        &cTypeDefs);

                    if (hr == S_OK)
                    {
                        for (UINT32 iTokenIndex = 0; iTokenIndex < cTypeDefs; ++iTokenIndex)
                        {
                            hr = spMetaDataImport->GetTypeDefProps(
                                rgTypeDefs[iTokenIndex],
                                pszRetrievedName,
                                ARRAYSIZE(pszRetrievedName),
                                nullptr,
                                &dwTypeDefProps,
                                nullptr);

                            if (FAILED(hr))
                            {
                                break;
                            }

                            hr = RO_E_METADATA_NAME_NOT_FOUND;

                            // Only consider windows runtime types when
                            // trying to determine if the name is a
                            // namespace.
                            if (IsTdWindowsRuntime(dwTypeDefProps) && (wcslen(pszRetrievedName) > cFullName))
                            {
                                if ((wcsncmp(pszRetrievedName, pszFullName, cFullName) == 0) &&
                                    (pszRetrievedName[cFullName] == L'.'))
                                {
                                    hr = RO_E_METADATA_NAME_IS_NAMESPACE;
                                    break;
                                }
                            }
                        }
                    }
                } while (hr == RO_E_METADATA_NAME_NOT_FOUND);

                // There were no more tokens to enumerate, but the type was still not found.
                if (hr == S_FALSE)
                {
                    hr = RO_E_METADATA_NAME_NOT_FOUND;
                }

                if (hEnum != nullptr)
                {
                    spMetaDataImport->CloseEnum(hEnum);
                    hEnum = nullptr;
                }
            }
        }
        return hr;
    }

    HRESULT FindTypeInDirectory(
        __in IMetaDataDispenserEx* pMetaDataDispenser,
        __in PCWSTR pszFullName,
        __in PCWSTR pszDirectoryPath,
        __out_opt HSTRING* phstrMetaDataFilePath,
        __deref_opt_out_opt IMetaDataImport2** ppMetaDataImport,
        __out_opt mdTypeDef* pmdTypeDef)
    {
        HRESULT hr;

        wchar_t szCandidateFilePath[MAX_PATH + 1] = { 0 };
        wchar_t szCandidateFileName[MAX_PATH + 1] = { 0 };
        PWSTR pszLastDot;

        hr = StringCchCopy(szCandidateFileName, ARRAYSIZE(szCandidateFileName), pszFullName);

        if (SUCCEEDED(hr))
        {
            // To resolve type SomeNamespace.B.C, first check if SomeNamespace.B.C is a type or
            // a namespace in the metadata files in the directory in this order:
            // 1. SomeNamespace.B.C.WinMD
            // 2. SomeNamespace.B.WinMD
            // 3. SomeNamespace.WinMD
            do
            {
                pszLastDot = nullptr;

                hr = StringCchPrintfEx(
                    szCandidateFilePath,
                    ARRAYSIZE(szCandidateFilePath),
                    nullptr,
                    nullptr,
                    0,
                    METADATA_FILE_PATH_FORMAT,
                    pszDirectoryPath,
                    szCandidateFileName);

                if (SUCCEEDED(hr))
                {
                    hr = FindTypeInMetaDataFile(
                        pMetaDataDispenser,
                        pszFullName,
                        szCandidateFilePath,
                        TRO_RESOLVE_TYPE_AND_NAMESPACE,
                        ppMetaDataImport,
                        pmdTypeDef);

                    if (SUCCEEDED(hr))
                    {
                        if (phstrMetaDataFilePath != nullptr)
                        {
                            hr = WindowsCreateString(
                                szCandidateFilePath,
                                static_cast<UINT32>(wcslen(szCandidateFilePath)),
                                phstrMetaDataFilePath);
                        }
                        break;
                    }
                }

                hr = RO_E_METADATA_NAME_NOT_FOUND;
                pszLastDot = wcsrchr(szCandidateFileName, '.');

                if (pszLastDot != nullptr)
                {
                    *pszLastDot = '\0';
                }
            } while (pszLastDot != nullptr);

            // If name was not found when searching in the "upward direction", then
            // the name might be a namespace name in a down-level file.
            if (hr == RO_E_METADATA_NAME_NOT_FOUND)
            {
                wchar_t szFilePathSearchTemplate[MAX_PATH + 1] = { 0 };

                hr = StringCchPrintfEx(
                    szFilePathSearchTemplate,
                    ARRAYSIZE(szFilePathSearchTemplate),
                    nullptr,
                    nullptr,
                    0,
                    METADATA_FILE_SEARCH_FORMAT,
                    pszDirectoryPath,
                    pszFullName);

                if (SUCCEEDED(hr))
                {
                    WIN32_FIND_DATA fd;
                    HANDLE hFindFile;

                    // Search in all files in the directory whose name begin with the input string.
                    hFindFile = FindFirstFile(szFilePathSearchTemplate, &fd);

                    if (hFindFile != INVALID_HANDLE_VALUE)
                    {
                        PWSTR pszFilePathPart;
                        size_t cchRemaining;

                        do
                        {
                            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                            {
                                continue;
                            }

                            pszFilePathPart = szCandidateFilePath;
                            cchRemaining = ARRAYSIZE(szCandidateFilePath);
                            hr = StringCchCopyEx(
                                pszFilePathPart,
                                cchRemaining,
                                pszDirectoryPath,
                                &pszFilePathPart,
                                &cchRemaining,
                                0);

                            if (SUCCEEDED(hr))
                            {
                                hr = StringCchCopyEx(
                                    pszFilePathPart,
                                    cchRemaining,
                                    fd.cFileName,
                                    &pszFilePathPart,
                                    &cchRemaining,
                                    0);
                            }

                            if (SUCCEEDED(hr))
                            {
                                hr = FindTypeInMetaDataFile(
                                    pMetaDataDispenser,
                                    pszFullName,
                                    szCandidateFilePath,
                                    TRO_RESOLVE_NAMESPACE,
                                    ppMetaDataImport,
                                    pmdTypeDef);

                                if (hr == S_OK)
                                {
                                    hr = E_UNEXPECTED;
                                    break;
                                }

                                if (hr == RO_E_METADATA_NAME_IS_NAMESPACE)
                                {
                                    break;
                                }
                            }
                        } while (FindNextFile(hFindFile, &fd));

                        FindClose(hFindFile);
                    }
                    else
                    {
                        hr = RO_E_METADATA_NAME_NOT_FOUND;
                    }
                }
            }
        }

        if (hr == STRSAFE_E_INSUFFICIENT_BUFFER)
        {
            hr = RO_E_METADATA_NAME_NOT_FOUND;
        }

        return hr;
    }

    HRESULT FindTypeInDirectoryWithNormalization(
        __in IMetaDataDispenserEx* pMetaDataDispenser,
        __in PCWSTR pszFullName,
        __in PCWSTR pszDirectoryPath,
        __out_opt HSTRING* phstrMetaDataFilePath,
        __deref_opt_out_opt IMetaDataImport2** ppMetaDataImport,
        __out_opt mdTypeDef* pmdTypeDef)
    {
        wchar_t pszPackagePath[MAX_PATH + 1];
        PWSTR pszPackagePathWritePtr = pszPackagePath;
        size_t cchPackagePathRemaining = ARRAYSIZE(pszPackagePath);

        HRESULT hr = StringCchCopyEx(
            pszPackagePath,
            ARRAYSIZE(pszPackagePath),
            pszDirectoryPath,
            &pszPackagePathWritePtr,
            &cchPackagePathRemaining,
            0);

        if (SUCCEEDED(hr))
        {
            // If the path is not terminated by a backslash, then append one.
            if (pszPackagePath[ARRAYSIZE(pszPackagePath) - cchPackagePathRemaining - 1] != L'\\')
            {
                hr = StringCchCopyEx(
                    pszPackagePathWritePtr,
                    cchPackagePathRemaining,
                    L"\\",
                    &pszPackagePathWritePtr,
                    &cchPackagePathRemaining,
                    0);
            }
        }

        if (SUCCEEDED(hr))
        {
            hr = FindTypeInDirectory(
                pMetaDataDispenser,
                pszFullName,
                pszPackagePath,
                phstrMetaDataFilePath,
                ppMetaDataImport,
                pmdTypeDef);
        }

        return hr;
    }

    HRESULT ResolveThirdPartyType(
        __in IMetaDataDispenserEx* pMetaDataDispenser,
        __in PCWSTR pszFullName,
        __out_opt HSTRING* phstrMetaDataFilePath,
        __deref_opt_out_opt IMetaDataImport2** ppMetaDataImport,
        __out_opt mdTypeDef* pmdTypeDef)
    {
        HRESULT hr = HRESULT_FROM_WIN32(APPMODEL_ERROR_NO_PACKAGE);
        UINT32 dwPackagesCount = 0;
        UINT32 dwBufferLength = 0;

        PCWSTR exeDir = nullptr;  // Never freed; owned by process global.
        RETURN_IF_FAILED(GetProcessExeDir(&exeDir));

        hr = FindTypeInDirectoryWithNormalization(
            pMetaDataDispenser,
            pszFullName,
            exeDir,
            phstrMetaDataFilePath,
            ppMetaDataImport,
            pmdTypeDef);

        if (hr == RO_E_METADATA_NAME_NOT_FOUND)
        {
            // For compatibility purposes, if we fail to find the type in the unpackaged location, we should return
            // HRESULT_FROM_WIN32(APPMODEL_ERROR_NO_PACKAGE) instead of a "not found" error. This preserves the
            // behavior that existed before unpackaged type resolution was implemented.
            hr = HRESULT_FROM_WIN32(APPMODEL_ERROR_NO_PACKAGE);
        }
    }
}