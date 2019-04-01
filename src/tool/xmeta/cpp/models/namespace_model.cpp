#include "namespace_model.h"

namespace xlang
{
    namespace xmeta
    {
#pragma region namespace_body_model
        bool namespace_body_model::member_id_exists(const std::string &member_id)
        {
            if (classes.find(member_id) != classes.end())
                return true;

            if (structs.find(member_id) != structs.end())
                return true;

            if (interfaces.find(member_id) != interfaces.end())
                return true;

            return false;
        }

        std::string namespace_body_model::get_full_namespace_name()
        {
            return containing_namespace->get_full_namespace_name();
        }
#pragma endregion


#pragma region namespace_model
        namespace_model::namespace_model(const std::string &id, const size_t &decl_line, const std::shared_ptr<namespace_model> &pn) :
            base_model{ id, decl_line },
            parent_namespace{ pn }
        { }

        bool namespace_model::member_id_exists(const std::string &member_id)
        {
            for (auto ns_body : ns_bodies)
            {
                if (ns_body->member_id_exists(member_id))
                {
                    return true;
                }
            }

            if (child_namespaces.find(member_id) != child_namespaces.end())
                return true;

            return false;
        }

        std::string namespace_model::get_full_namespace_name()
        {
            if (parent_namespace != nullptr)
                return parent_namespace->get_full_namespace_name() + "." + id;
            else return id;
        }
#pragma endregion namespace_model
    }
}
