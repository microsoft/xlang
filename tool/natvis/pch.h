#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include <vsdebugeng.h>
#include <vsdebugeng.templates.h>
#include <Dia2.h>
#include "winrt\base.h"
#include <rometadataapi.h>
#include <rometadata.h>
#include <filesystem>
#include <optional>
#include <set>
#include <memory>
#include <variant>
#include <cmd_reader.h>
#include <meta_reader.h>

#ifndef IF_FAIL_RET
#define IF_FAIL_RET(expr) { HRESULT _hr = (expr); if(FAILED(_hr)) { return(_hr); } }
#endif

template<typename T>
winrt::com_ptr<T> make_com_ptr(T* ptr)
{
    winrt::com_ptr<T> result;
    result.copy_from(ptr);
    return result;
}

extern std::unique_ptr<xlang::meta::reader::cache> db;
void load_type_winmd(WCHAR const* processPath, std::string runtimeTypeName);
