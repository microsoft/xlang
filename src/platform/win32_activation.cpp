#include "win32_pal_internal.h"
#include "platform_activation.h"

using namespace std::string_view_literals;

namespace xlang::impl
{
    xlang_pfn_lib_get_activation_factory try_get_activation_func(
        std::basic_string_view<wchar_t> module_namespace)
    {
        constexpr auto file_ext{ L".dll"sv };
        if (module_namespace.size() + file_ext.size() >= MAX_PATH)
        {
            // Such a long name wouldn't be a valid file.
            return nullptr;
        }
        wchar_t module_name[MAX_PATH];
        wchar_t* out = std::copy(module_namespace.begin(), module_namespace.end(), module_name);
        out = std::copy(file_ext.begin(), file_ext.end(), out);
        *out = L'\0';

        HMODULE module = ::LoadLibraryW(module_name);
        if (module)
        {
            return reinterpret_cast<xlang_pfn_lib_get_activation_factory>(::GetProcAddress(module, activation_fn_name.data()));
        }
        return nullptr;
    }
}
