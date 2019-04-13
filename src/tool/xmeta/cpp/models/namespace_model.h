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
        namespace_body_model(std::shared_ptr<namespace_model> const& containing_namespace);

        void add_using_alias_directive(std::shared_ptr<using_alias_directive_model> const& uad);
        void add_using_namespace_directive(using_namespace_directive_model&& und);
        void add_class(std::shared_ptr<class_model> const& cm);
        void add_struct(std::shared_ptr<struct_model> const& sm);
        void add_interface(std::shared_ptr<interface_model> const& im);
        void add_enum(enum_model&& em);
        void add_delegate(delegate_model&& em);

        bool member_id_exists(std::string_view const& member_id);
        std::string get_full_namespace_name();

    private:
        // Using directives
        std::map<std::string_view, std::shared_ptr<using_alias_directive_model>, std::less<>> m_using_alias_directives;
        std::vector<using_namespace_directive_model> m_using_namespace_directives;

        // Members
        std::map<std::string_view, std::shared_ptr<class_model>, std::less<>> m_classes;
        std::map<std::string_view, std::shared_ptr<struct_model>, std::less<>> m_structs;
        std::map<std::string_view, std::shared_ptr<interface_model>, std::less<>> m_interfaces;
        std::vector<enum_model> m_enums;
        std::vector<delegate_model> m_delegates;

        std::shared_ptr<namespace_model> m_containing_namespace;
    };

    struct namespace_model : base_model
    {
        namespace_model(std::string_view const& id, size_t decl_line, std::shared_ptr<namespace_model> const& parent);
        namespace_model() = delete;

        auto const& get_parent_namespace() const;

        void add_child_namespace(std::shared_ptr<namespace_model> child);
        void add_namespace_body(std::shared_ptr<namespace_body_model> body);

        // Used for semantic check #3 for namespace members
        bool member_id_exists(std::string_view const& member_id);
        std::string get_full_namespace_name();

    private:
        std::shared_ptr<namespace_model> m_parent_namespace;
        std::map<std::string_view, std::shared_ptr<namespace_model>, std::less<>> m_child_namespaces;
        std::vector<std::shared_ptr<namespace_body_model>> m_namespace_bodies;
    };
}
