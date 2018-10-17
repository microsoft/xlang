#include "pal_internal.h"
#include "opaque_string_wrapper.h"
#include "platform_activation.h"

namespace xlang::impl
{
    template <typename char_type>
    void* get_activation_factory(
        xlang_string class_name,
        xlang_guid const& iid)
    {
        for (auto current_namespace = enclosing_namespace(to_string_view<filesystem_char_type>(class_name));
            !current_namespace.empty();
            current_namespace = enclosing_namespace(current_namespace))
        {
            xlang_pfn_lib_get_activation_factory pfn = try_get_activation_func(current_namespace);
            if (pfn)
            {
                void* factory{};
                xlang_result result = (*pfn)(class_name, iid, &factory);
                if (result == xlang_error_ok)
                {
                    return factory;
                }
                else if (result != xlang_error_class_not_available)
                {
                    throw_result(result);
                }
            }
        }
        throw_result(xlang_error_class_not_available);
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
    *factory = get_activation_factory<filesystem_char_type>(class_name, iid);

    return xlang_error_ok;
}
catch (...)
{
    *factory = nullptr;
    return xlang::to_result();
}