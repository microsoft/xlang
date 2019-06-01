#pragma once

#include <assert.h>
#include <string_view>
#include <vector>
#include <unordered_set>

#include "class_or_interface_model.h"

namespace xlang::xmeta
{
    std::shared_ptr<interface_model> create_interface_from(std::shared_ptr<xlang::meta::reader::TypeDef> type_def);

    struct interface_model : class_or_interface_model
    {
        interface_model() = delete;
        interface_model(std::string_view const& name, size_t decl_line, std::string_view const& assembly_name, std::shared_ptr<namespace_body_model> const& containing_ns_body, bool is_synthesized = false) :
            class_or_interface_model{ name, decl_line, assembly_name, containing_ns_body }, is_synthesized{ is_synthesized }
        { }
        interface_model(std::string_view const& name, size_t decl_line, std::string_view const& assembly_name, std::string_view const& containing_ns_name, bool is_synthesized = false) :
            class_or_interface_model{ name, decl_line, assembly_name, containing_ns_name }, is_synthesized{ is_synthesized }
        { }

        void validate(xlang_error_manager & error_manager);

        void resolve(symbol_table & symbols, xlang_error_manager & error_manager);

        bool has_circular_inheritance(xlang_error_manager & error_manager);

        bool is_synthesized = false;

    private:
        bool has_circular_inheritance(std::set<std::string> & symbol_set, xlang_error_manager & error_manager);
    };
}
