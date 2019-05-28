#pragma once

#include <memory>
#include <string_view>
#include <set>
#include "base_model.h"
#include "namespace_member_model.h"

namespace xlang::xmeta
{
    struct class_or_interface_model : namespace_member_model
    {
        friend interface_model;
        friend class_model;

        class_or_interface_model() = delete;

        class_or_interface_model(std::string_view const& id, size_t decl_line, std::string_view const& assembly_name, std::shared_ptr<namespace_body_model> const& containing_ns_body) :
            namespace_member_model{ id, decl_line, assembly_name, containing_ns_body }
        { }

        class_or_interface_model(std::string_view const& id, size_t decl_line, std::string_view const& assembly_name, std::string_view const& containing_namespace_name) :
            namespace_member_model{ id, decl_line, assembly_name, containing_namespace_name }
        { }

        auto const& get_interface_bases() const noexcept
        {
            return m_interface_base_refs;
        }

        auto const& get_properties() const noexcept
        {
            return m_properties;
        }

        auto const& get_methods() const noexcept
        {
            return m_methods;
        }

        auto const& get_events() const noexcept
        {
            return m_events;
        }

        void add_interface_base_ref(std::string_view const& interface_base_ref);

        compilation_error add_member(std::shared_ptr<property_model> const& member);

        compilation_error add_member(std::shared_ptr<method_model> const& member);

        compilation_error add_member(std::shared_ptr<event_model> const& member);

        std::shared_ptr<property_model> const& get_property_member(std::string const& member_id);

        bool member_id_exists(std::string_view const& id);

        bool property_id_exists(std::string_view const& id);

        void validate(xlang_error_manager & error_manager);

        void resolve(symbol_table & symbols, xlang_error_manager & error_manager);

    private:
        std::vector<type_ref> m_interface_base_refs;
        // TODO: Add type parameters (generic types)

        bool contains_itself = false;

        // Members
        std::vector<std::shared_ptr<property_model>> m_properties;
        std::vector<std::shared_ptr<method_model>> m_methods;
        std::vector<std::shared_ptr<event_model>> m_events;

        std::set<std::shared_ptr<interface_model>> get_all_interface_bases();
    };
}
