#pragma once

#include <string>
#include <string_view>
#include <utility>
#include <set>
#include <vector>
#include <variant>

#include "base_model.h"
#include "xlang_error.h"
#include "namespace_member_model.h"

namespace xlang::xmeta
{
    struct struct_model : namespace_member_model
    {
        struct_model() = delete;
        struct_model(std::string_view const& id, size_t decl_line, 
                    std::string_view const& assembly_name, 
                    std::shared_ptr<namespace_body_model> const& containing_ns_body) :
            namespace_member_model{ id, decl_line, assembly_name, containing_ns_body }
        { }

        struct_model(std::string_view const& id, size_t decl_line,
            std::string_view const& assembly_name,
            std::string_view const& containing_ns_name) :
            namespace_member_model{ id, decl_line, assembly_name, containing_ns_name }
        { }

        auto const& get_fields() const noexcept
        {
            return m_fields;
        }

        void add_field(std::pair<type_ref, std::string>&& field)
        {
            m_fields.emplace_back(std::move(field));
        }

        void resolve(symbol_table & symbols, xlang_error_manager & error_manager);

        bool has_circular_struct_declarations(xlang_error_manager & error_manager);

        bool has_circular_struct_declarations(std::set<std::string> & symbol_set, xlang_error_manager & error_manager);

        bool member_exists(std::string_view const& id) const
        {
            auto same_id = [&id](std::pair<type_ref, std::string> const& field)
            {
                return field.second == id;
            };
            return std::find_if(m_fields.begin(), m_fields.end(), same_id) != m_fields.end();
        }

    private:
        std::vector<std::pair<type_ref, std::string>> m_fields;
        bool contains_itself = false;
    };
}
