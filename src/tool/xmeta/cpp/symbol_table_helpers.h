#pragma once

#include <algorithm>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>

#include "models/xlang_models.h"

namespace xlang::xmeta
{
    class symbol_table_helper
    {
    public:
        void write_error(size_t decl_line, std::string_view const& msg);
        void write_class_dup_modifier_error(size_t decl_line, std::string_view const& mod_name, std::string_view const& class_name);
        void write_class_member_dup_modifier_error(size_t decl_line, std::string_view const& mod_name, std::string_view const& member_type, std::string_view const& id);
        void write_enum_member_name_error(size_t decl_line, std::string_view const& invalid_name, std::string_view const& enum_name);
        void write_enum_member_expr_ref_error(size_t decl_line, std::string_view const& invalid_name, std::string_view const& enum_name);
        void write_enum_const_expr_range_error(size_t decl_line, std::string_view const& invalid_expr, std::string_view const& enum_name);
        void write_event_dup_modifier_error(size_t decl_line, std::string_view const& mod_name, std::string_view const& event_name);
        void write_namespace_name_error(size_t decl_line, std::string_view const& invalid_name, std::string_view const& original_name);
        void write_namespace_member_name_error(size_t decl_line, std::string_view const& invalid_name);
        void write_using_alias_directive_name_error(size_t decl_line, std::string_view const& invalid_name);
        bool semantic_error_exists = false;

        // Pushes a namespace to the current namespace scope, and adds it to the symbol table if necessary.
        void push_namespace(const std::string& name, const size_t& decl_line);

        // Pops a namespace from the namespace scope.
        void pop_namespace();

        auto const& get_namespaces() const
        {
            return namespaces;
        }

        auto const& get_cur_namespace_body() const
        {
            return cur_namespace_body;
        }
        void set_cur_namespace_body(std::shared_ptr<class_model> const& cur_namespace_body)
        {
            this->cur_namespace_body = cur_namespace_body;
        }

        auto const& get_cur_class() const
        {
            return cur_class;
        }
        void set_cur_class(std::shared_ptr<class_model> const& cur_class)
        {
            this->cur_class = cur_class;
        }

        auto const& get_cur_interface() const
        {
            return cur_interface;
        }
        void set_cur_interface(std::shared_ptr<class_model> const& cur_interface)
        {
            this->cur_interface = cur_interface;
        }

        auto const& get_cur_struct() const
        {
            return cur_struct;
        }
        void set_cur_struct(std::shared_ptr<class_model> const& cur_struct)
        {
            this->cur_class = cur_struct;
        }

    private:
        std::map<std::string_view, std::shared_ptr<namespace_model>, std::less<>> namespaces;
        std::shared_ptr<namespace_body_model> cur_namespace_body;
        std::shared_ptr<class_model> cur_class;
        std::shared_ptr<interface_model> cur_interface;
        std::shared_ptr<struct_model> cur_struct;
        
    };
    std::string to_lower_copy(std::string_view sv);
}
