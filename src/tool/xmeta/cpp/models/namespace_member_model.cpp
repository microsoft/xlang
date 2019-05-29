#include "namespace_member_model.h"
#include "namespace_model.h"

namespace xlang::xmeta
{
    std::string const namespace_member_model::get_fully_qualified_id() const noexcept
    {
        if (containing_namespace_name)
        {
            return containing_namespace_name.value() + "." + get_name();
        }
        return get_containing_namespace_body()->get_containing_namespace()->get_fully_qualified_id() + "." + get_name();
    }
}