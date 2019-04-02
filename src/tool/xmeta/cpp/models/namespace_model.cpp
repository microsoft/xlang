#include "namespace_model.h"

namespace xlang::xmeta
{
#pragma region namespace_body_model
    bool namespace_body_model::member_id_exists(std::string_view const& member_id)
    {
        return classes.find(member_id) != classes.end() ||
            structs.find(member_id) != structs.end() ||
            interfaces.find(member_id) != interfaces.end();
    }

    std::string namespace_body_model::get_full_namespace_name()
    {
        return containing_namespace->get_full_namespace_name();
    }
#pragma endregion


#pragma region namespace_model
    namespace_model::namespace_model(std::string_view const& id, size_t decl_line, std::shared_ptr<namespace_model> const& parent) :
        base_model{ id, decl_line },
        parent_namespace{ parent }
    { }

    bool namespace_model::member_id_exists(std::string_view const& member_id)
    {
        for (auto ns_body : ns_bodies)
        {
            if (ns_body->member_id_exists(member_id))
            {
                return true;
            }
        }

        return child_namespaces.find(member_id) != child_namespaces.end();
    }

    std::string namespace_model::get_full_namespace_name()
    {
        if (parent_namespace != nullptr)
        {
            return parent_namespace->get_full_namespace_name() + "." + std::string(id);
        }
        else
        {
            return std::string(id);
        }
    }
#pragma endregion namespace_model
}
