#pragma once

#include <algorithm>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>

#include "antlr4-runtime.h"
#include "XlangParserBaseListener.h"

#include "models/xmeta_models.h"

struct ast_to_st_listener;

namespace xlang::xmeta
{
    struct xmeta_idl_reader
    {
        xmeta_idl_reader(std::string_view const& idl_assembly_name)
        {
            m_assembly_names.emplace_back(idl_assembly_name);
            m_cur_assembly = m_assembly_names.back();
        }
        friend struct ast_to_st_listener;

        size_t read(std::istream& idl_contents, bool disable_error_reporting = false);

        size_t read(std::istream& idl_contents, XlangParserBaseListener& listener, bool disable_error_reporting = false);

        void reset(std::string_view const& assembly_name);

        auto const& get_namespaces() const
        {
            return m_namespaces;
        }

        auto const& get_cur_namespace_body() const
        {
            return m_cur_namespace_body;
        }

        auto const& get_cur_assembly() const
        {
            return m_cur_assembly;
        }

        void set_cur_namespace_body(std::shared_ptr<namespace_body_model> const& cur_namespace_body)
        {
            this->m_cur_namespace_body = cur_namespace_body;
        }
        auto get_num_semantic_errors() const
        {
            return m_num_semantic_errors;
        }

    private:
        std::map<std::string_view, std::shared_ptr<namespace_model>, std::less<>> m_namespaces;
        std::shared_ptr<namespace_body_model> m_cur_namespace_body;
        std::shared_ptr<class_model> m_cur_class;
        std::shared_ptr<interface_model> m_cur_interface;
        std::shared_ptr<struct_model> m_cur_struct;

        std::string_view m_cur_assembly;
        std::vector<std::string> m_assembly_names;

        size_t m_num_semantic_errors = 0;

        // Pushes a namespace to the current namespace scope, and adds it to the symbol table if necessary.
        void push_namespace(std::string_view const& name, size_t decl_line);

        // Pops a namespace from the namespace scope.
        void pop_namespace();

        void write_error(size_t decl_line, std::string_view const& msg);

        void write_duplicate_modifier_error(size_t decl_line, std::string_view const& modifier_name, std::string_view const& member_name);
        void write_enum_member_name_error(size_t decl_line, std::string_view const& invalid_name, std::string_view const& enum_name);
        void write_enum_member_expr_ref_error(size_t decl_line, std::string_view const& invalid_name, std::string_view const& enum_name);
        void write_enum_circular_dependency(size_t decl_line, std::string_view const& invalid_member_id, std::string_view const& enum_name);
        void write_enum_const_expr_range_error(size_t decl_line, std::string_view const& invalid_expr, std::string_view const& enum_name);
        void write_namespace_name_error(size_t decl_line, std::string_view const& invalid_name, std::string_view const& original_name);
        void write_namespace_member_name_error(size_t decl_line, std::string_view const& invalid_name);
        void write_property_duplicate_get_error(size_t decl_line, std::string_view const& container_name, std::string_view const& property_name);
        void write_property_duplicate_set_error(size_t decl_line, std::string_view const& container_name, std::string_view const& property_name);

        inline std::string const get_cur_ns_name() const;
    };
    std::string copy_to_lower(std::string_view sv);
}
