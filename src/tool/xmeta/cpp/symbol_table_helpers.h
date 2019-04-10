#pragma once

#include <algorithm>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>

#include "models/class_model.h"
#include "models/interface_model.h"
#include "models/namespace_model.h"
#include "models/struct_model.h"

namespace xlang::xmeta
{
    extern std::map<std::string_view, std::shared_ptr<namespace_model>, std::less<>> namespaces;
    extern std::shared_ptr<namespace_body_model> cur_namespace_body;
    extern std::shared_ptr<class_model> cur_class;
    extern std::shared_ptr<interface_model> cur_interface;
    extern std::shared_ptr<struct_model> cur_struct;

    extern bool semantic_error_exists;

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

    std::string to_lower_copy(std::string_view sv);

    // Pushes a namespace to the current namespace scope, and adds it to the symbol table if necessary.
    void push_namespace(const std::string& name, const size_t& decl_line);

    // Pops a namespace from the namespace scope.
    void pop_namespace();
}
