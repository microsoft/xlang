#pragma once

#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "base_model.h"
#include "class_model.h"
#include "struct_model.h"


namespace xlang::xmeta
{
    struct namespace_body_model;
    struct namespace_model;

    // The scope of a using directive extends over the namespace member declarations of
    // its immediately containing namespace body. The namespace_body_model struct
    // will be used to differentiate the separate bodies.
    struct namespace_body_model
    {
        // Using directives
        std::map<std::string_view, std::string_view> using_alias_directives;
        std::vector<std::string_view> using_namespace_directives;

        // Members
        std::map<std::string_view, std::shared_ptr<class_model>> classes;
        std::map<std::string_view, std::shared_ptr<struct_model>> structs;
        std::map<std::string_view, std::shared_ptr<interface_model>> interfaces;

        std::shared_ptr<namespace_model> containing_namespace;

        // Methods
        bool member_id_exists(std::string_view const& member_id);
        std::string get_full_namespace_name();
    };

    struct namespace_model : base_model
    {
        namespace_model(std::string_view const& id, size_t decl_line, std::shared_ptr<namespace_model> const& parent);
        namespace_model() = delete;

        // Used for semantic check #3 for namespace members
        bool member_id_exists(std::string_view const& member_id);
        std::string get_full_namespace_name();

        std::shared_ptr<namespace_model> parent_namespace;

        // Members
        std::map<std::string_view, std::shared_ptr<namespace_model>> child_namespaces;

        // Vector of different namespace bodies defined.
        std::vector<std::shared_ptr<namespace_body_model>> ns_bodies;
    };
}
