#include <cassert>

#include "namespace_model.h"

namespace xlang::xmeta
{
    namespace_body_model::namespace_body_model(std::shared_ptr<namespace_model> const& containing_namespace) :
        m_containing_namespace{ containing_namespace }
    { }

    void namespace_body_model::add_using_alias_directive(std::shared_ptr<using_alias_directive_model> const& uad)
    {
        std::string_view id{ uad->get_id() };
        m_using_alias_directives[id] = uad;
    }

    void namespace_body_model::add_using_namespace_directive(using_namespace_directive_model&& und)
    {
        m_using_namespace_directives.emplace_back(std::move(und));
    }

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

    void namespace_body_model::add_enum(enum_model&& em)
    {
        m_enums.emplace_back(std::move(em));
    }

    void namespace_body_model::add_delegate(delegate_model&& dm)
    {
        m_delegates.emplace_back(std::move(dm));
    }

    bool namespace_body_model::member_id_exists(std::string_view const& member_id)
    {
        return m_classes.find(member_id) != m_classes.end() ||
            m_structs.find(member_id) != m_structs.end() ||
            m_interfaces.find(member_id) != m_interfaces.end();
    }

    std::string namespace_body_model::get_full_namespace_name()
    {
        return m_containing_namespace->get_full_namespace_name();
    }

    namespace_model::namespace_model(std::string_view const& id, size_t decl_line, std::shared_ptr<namespace_model> const& parent) :
        base_model{ id, decl_line },
        m_parent_namespace{ parent }
    { }

    auto const& namespace_model::get_parent_namespace() const
    {
        return m_parent_namespace;
    }

    void namespace_model::add_child_namespace(std::shared_ptr<namespace_model> child)
    {
        std::string const& id = child->get_id();
        assert(m_child_namespaces.find(id) == m_child_namespaces.end());
        m_child_namespaces[id] = child;
    }

    void namespace_model::add_namespace_body(std::shared_ptr<namespace_body_model> body)
    {
        m_namespace_bodies.emplace_back(body);
    }

    bool namespace_model::member_id_exists(std::string_view const& member_id)
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

    std::string namespace_model::get_full_namespace_name()
    {
        if (m_parent_namespace != nullptr)
        {
            return m_parent_namespace->get_full_namespace_name() + "." + std::string(get_id());
        }
        else
        {
            return std::string(get_id());
        }
    }
}
