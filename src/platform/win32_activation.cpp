#include "win32_pal_internal.h"
#include "platform_activation.h"

namespace xlang::impl
{
    xlang_pfn_lib_get_activation_factory try_get_activation_func(
        std::basic_string_view<wchar_t> module_namespace)
    {
        HMODULE module = ::LoadLibraryW(module_namespace.data());
        if (module)
        {
            return reinterpret_cast<xlang_pfn_lib_get_activation_factory>(::GetProcAddress(module, activation_fn_name.data()));
        }
        return nullptr;
    }
}
