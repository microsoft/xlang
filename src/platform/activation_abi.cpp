#include <experimental/filesystem>
#include "pal_internal.h"
#include "opaque_string_wrapper.h"

namespace xlang::impl
{
    template <typename char_type>
    void* get_activation_factory(
        xlang_string class_name,
        xlang_guid const& iid)
    {
        auto current_namespace = to_string_view<char_type>(class_name);
        
        for (auto separator_pos = current_namespace.rfind('.');
            separator_pos != current_namespace.npos;
            separator_pos = current_namespace.rfind('.'))
        {
            current_namespace = current_namespace.substr(0, separator_pos);
            xlang_pfn_lib_get_activation_factory pfn = nullptr;
        }

        throw_result(-1);
    }
}

using namespace xlang::impl;

XLANG_PAL_EXPORT xlang_result XLANG_CALL xlang_get_activation_factory(
    xlang_string class_name,
    xlang_guid const& iid,
    void** factory
) XLANG_NOEXCEPT
try
{
    if (!class_name)
    {
        xlang::throw_result(xlang_error_invalid_arg);
    }

    // Use the platform's preferred filename encoding
    *factory = get_activation_factory<std::experimental::filesystem::path::value_type>(class_name, iid);

    return xlang_error_ok;
}
catch (...)
{
    *factory = nullptr;
    return xlang::to_result();
}