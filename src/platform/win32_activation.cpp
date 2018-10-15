#include <Windows.h>
#include <experimental/filesystem>
#include "platform_activation.h"

#if !XLANG_PLATFORM_WINDOWS
#error "This file is only for targeting Windows"
#endif

using namespace std::experimental::filesystem;

namespace xlang::impl
{
    xlang_pfn_lib_get_activation_factory try_get_activation_func(
        std::basic_string_view<wchar_t> module_name)
    {
        wchar_t exe_filename[MAX_PATH];
        ::GetModuleFileNameW(nullptr, exe_filename, MAX_PATH);

        path p{ exe_filename };

        return nullptr;
    }
}
