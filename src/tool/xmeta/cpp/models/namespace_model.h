#pragma once

#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "base_model.h"
#include "class_model.h"
#include "delegate_model.h"
#include "enum_model.h"
#include "struct_model.h"
#include "using_directive_models.h"


namespace xlang::xmeta
{
    struct namespace_body_model;
    struct namespace_model;

    // The scope of a using directive extends over the namespace member declarations of
    // its immediately containing namespace body. The namespace_body_model struct
    // will be used to differentiate the separate bodies.
    struct namespace_body_model
    {
        namespace_body_model() = delete;
        namespace_body_model(std::shared_ptr<namespace_model> const& containing_namespace) :
            m_containing_namespace{ containing_namespace }
        { }

        auto const& get_using_alias_directives() const noexcept
        {
            return m_using_alias_directives;
        }

        auto const& get_using_namespace_directives() const noexcept
        {
            return m_using_namespace_directives;
        }

        auto const& get_classes() const noexcept
        {
            return m_classes;
        }

        auto const& get_structs() const noexcept
        {
            return m_structs;
        }

        auto const& get_interfaces() const noexcept
        {
            return m_interfaces;
        }

        auto const& get_enums() const noexcept
        {
            return m_enums;
        }

        auto const& get_delegates() const noexcept
        {
            return m_delegates;
        }

        auto const& get_containing_namespace() const noexcept
        {
            return m_containing_namespace;
        }

        void add_using_alias_directive(std::shared_ptr<using_alias_directive_model> const& uad)
        {
            std::string_view id{ uad->get_id() };
            m_using_alias_directives[id] = uad;
        }

        void add_using_namespace_directive(using_namespace_directive_model&& und)
        {
            m_using_namespace_directives.emplace_back(std::move(und));
        }

        void add_class(std::shared_ptr<class_model> const& cm)
        {
            m_classes[cm->get_id()] = cm;
        }

        void add_struct(std::shared_ptr<struct_model> const& sm)
        {
            m_structs[sm->get_id()] = sm;
        }

        void add_interface(std::shared_ptr<interface_model> const& im)
        {
            m_interfaces[im->get_id()] = im;
        }

        void add_enum(std::shared_ptr<enum_model> const& em)
        {
            m_enums.emplace_back(em);
        }

        void add_delegate(delegate_model&& dm)
        {
            m_delegates.emplace_back(std::move(dm));
        }

        bool member_id_exists(std::string_view const& member_id) const
        {
            return m_classes.find(member_id) != m_classes.end() ||
                m_structs.find(member_id) != m_structs.end() ||
                m_interfaces.find(member_id) != m_interfaces.end();
        }

    private:
        // Using directives
        std::map<std::string_view, std::shared_ptr<using_alias_directive_model>, std::less<>> m_using_alias_directives;
        std::vector<using_namespace_directive_model> m_using_namespace_directives;

        // Members
        std::map<std::string_view, std::shared_ptr<class_model>, std::less<>> m_classes;
        std::map<std::string_view, std::shared_ptr<struct_model>, std::less<>> m_structs;
        std::map<std::string_view, std::shared_ptr<interface_model>, std::less<>> m_interfaces;
        std::vector<std::shared_ptr<enum_model>> m_enums;
        std::vector<delegate_model> m_delegates;

        std::shared_ptr<namespace_model> m_containing_namespace;
    };

    struct namespace_model : base_model
    {
        namespace_model() = delete;
        namespace_model(std::string_view const& id, size_t decl_line, std::string_view const& assembly_name, std::shared_ptr<namespace_model> const& parent) :
            base_model{ id, decl_line, assembly_name },
            m_parent_namespace{ parent }
        { }

        auto const& get_child_namespaces() const noexcept
        {
            return m_child_namespaces;
        }

        auto const& get_namespace_bodies() const noexcept
        {
            return m_namespace_bodies;
        }

        auto const& get_parent_namespace() const noexcept
        {
            return m_parent_namespace;
        }

        void add_child_namespace(std::shared_ptr<namespace_model> const& child)
        {
            auto id = child->get_id();
            assert(m_child_namespaces.find(id) == m_child_namespaces.end());
            m_child_namespaces[id] = child;
        }

        void add_namespace_body(std::shared_ptr<namespace_body_model> const& body)
        {
            m_namespace_bodies.emplace_back(body);
        }

        // Used for semantic check #3 for namespace members
        bool member_id_exists(std::string_view const& member_id) const
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

        std::string get_full_name() const
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

    private:
        std::shared_ptr<namespace_model> m_parent_namespace;
        std::map<std::string_view, std::shared_ptr<namespace_model>, std::less<>> m_child_namespaces;
        std::vector<std::shared_ptr<namespace_body_model>> m_namespace_bodies;
    };
}
