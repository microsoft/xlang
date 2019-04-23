#include "namespace_model.h"

#include "using_directive_models.h"

#include "class_model.h"
#include "interface_model.h"
#include "property_model.h"
#include "struct_model.h"
#include "enum_model.h"

namespace xlang::xmeta
{
    void namespace_body_model::add_class(std::shared_ptr<class_model> const& cm)
    {
        m_classes[cm->get_id()] = cm;
    }

    void namespace_body_model::add_struct(std::shared_ptr<struct_model> const& sm)
    {
        m_structs[sm->get_id()] = sm;
    }

    void namespace_body_model::add_interface(std::shared_ptr<interface_model> const& im)
    {
        m_interfaces[im->get_id()] = im;
    }

    void namespace_body_model::add_enum(std::shared_ptr<enum_model> const& em)
    {
        m_enums.emplace_back(em);
    }

    void namespace_body_model::add_delegate(std::shared_ptr<delegate_model> const& dm)
    {
        m_delegates.emplace_back(dm);
    }

    void namespace_body_model::add_using_alias_directive(std::shared_ptr<using_alias_directive_model> const& uad)
    {
        std::string_view id{ uad->get_id() };
        m_using_alias_directives[id] = uad;
    }

    void namespace_body_model::add_using_namespace_directive(std::shared_ptr<using_namespace_directive_model> const& und)
    {
        m_using_namespace_directives.emplace_back(std::move(und));
    }

    bool namespace_body_model::member_id_exists(std::string_view const& member_id) const
    {
        return m_classes.find(member_id) != m_classes.end() ||
            m_structs.find(member_id) != m_structs.end() ||
            m_interfaces.find(member_id) != m_interfaces.end();
    }


    void namespace_model::add_child_namespace(std::shared_ptr<namespace_model> const& child)
    {
        auto id = child->get_id();
        assert(m_child_namespaces.find(id) == m_child_namespaces.end());
        m_child_namespaces[id] = child;
    }

    void namespace_model::add_namespace_body(std::shared_ptr<namespace_body_model> const& body)
    {
        m_namespace_bodies.emplace_back(body);
    }

    bool namespace_model::member_id_exists(std::string_view const& member_id) const
    {
        for (auto ns_body : m_namespace_bodies)
        {
            if (ns_body->member_id_exists(member_id))
            {
                return true;
            }
        }

        return m_child_namespaces.find(member_id) != m_child_namespaces.end();
    }

    std::string namespace_model::get_full_name() const
    {
        if (m_parent_namespace != nullptr)
        {
            return m_parent_namespace->get_full_name() + "." + get_id();
        }
        else
        {
            return get_id();
        }
    }
}
