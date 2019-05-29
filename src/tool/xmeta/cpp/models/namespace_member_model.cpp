#include "namespace_member_model.h"
#include "namespace_model.h"

namespace xlang::xmeta
{
    std::string const namespace_member_model::get_qualified_name() const noexcept
    {
        if (containing_namespace_name)
        {
            return containing_namespace_name.value() + "." + get_name();
        }
        return get_containing_namespace_body()->get_containing_namespace()->get_qualified_name() + "." + get_name();
    }
}