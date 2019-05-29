#pragma once

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "base_model.h"

namespace xlang::xmeta
{
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

        void add_using_alias_directive(std::shared_ptr<using_alias_directive_model> const& uad);
        void add_using_namespace_directive(std::shared_ptr<using_namespace_directive_model> const& und);

        void add_class(std::shared_ptr<class_model> const& cm);
        void add_struct(std::shared_ptr<struct_model> const& sm);
        void add_interface(std::shared_ptr<interface_model> const& im);
        void add_enum(std::shared_ptr<enum_model> const& em);
        void add_delegate(std::shared_ptr<delegate_model> const& dm);

        bool type_id_exists(std::string_view const& member_id) const;

    private:
        // Using directives
        std::map<std::string_view, std::shared_ptr<using_alias_directive_model>, std::less<>> m_using_alias_directives;
        std::vector<std::shared_ptr<using_namespace_directive_model>> m_using_namespace_directives;

        // Members
        std::map<std::string_view, std::shared_ptr<class_model>, std::less<>> m_classes;
        std::map<std::string_view, std::shared_ptr<struct_model>, std::less<>> m_structs;
        std::map<std::string_view, std::shared_ptr<interface_model>, std::less<>> m_interfaces;
        std::map<std::string_view, std::shared_ptr<enum_model>, std::less<>> m_enums;
        std::map<std::string_view, std::shared_ptr<delegate_model>, std::less<>> m_delegates;

        std::shared_ptr<namespace_model> m_containing_namespace;
    };

    struct namespace_model : base_model
    {
        namespace_model() = delete;
        namespace_model(
            std::string_view const& id,
            size_t decl_line,
            std::string_view const& assembly_name,
            std::shared_ptr<namespace_model> const& parent) :
            base_model{ id, decl_line, assembly_name },
            m_fully_qualified_id{ parent != nullptr
                ? parent->get_qualified_name() + "." + std::string(id)
                : id },
            m_parent_namespace { parent }
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

        std::string const& get_qualified_name() const noexcept
        {
            return m_fully_qualified_id;
        }

        void add_namespace_body(std::shared_ptr<namespace_body_model> const& body);
        void add_child_namespace(std::shared_ptr<namespace_model> const& child);

        // Used for semantic check #3 for namespace members
        bool member_id_exists(std::string_view const& member_id) const;
        bool child_namespace_exists(std::string_view const& member_id) const;

    private:
        std::map<std::string_view, std::shared_ptr<namespace_model>, std::less<>> m_child_namespaces;
        std::vector<std::shared_ptr<namespace_body_model>> m_namespace_bodies;
        std::shared_ptr<namespace_model> m_parent_namespace;
        std::string m_fully_qualified_id;
    };
}
