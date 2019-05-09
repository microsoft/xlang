#pragma once

#include <string>
#include <string_view>
#include <utility>
#include <set>
#include <vector>
#include <variant>
#include "model_types.h"
#include "namespace_member_model.h"
#include "property_model.h"
#include "xlang_error.h"

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

        auto const& get_fields() const noexcept
        {
            return m_fields;
        }

        void add_field(std::pair<type_ref, std::string>&& field)
        {
            m_fields.emplace_back(std::move(field));
        }

        void resolve(std::map<std::string, class_type_semantics> const& symbols, xlang_error_manager & error_manager)
        {
            for (auto & field : m_fields)
            {
                type_ref & field_type = field.first;
                if (!field_type.get_semantic().is_resolved())
                {
                    // TODO: Once we have using directives, we will need to go through many fully_qualified_ids here
                    std::string ref_name = field_type.get_semantic().get_ref_name();
                    std::string symbol = ref_name.find(".") != std::string::npos 
                        ? ref_name : this->get_containing_namespace_body()->get_containing_namespace()->get_fully_qualified_id() + "." + ref_name;

                    auto iter = symbols.find(symbol);
                    if (iter == symbols.end())
                    {
                        error_manager.write_unresolved_type_error(get_decl_line(), symbol);
                    }
                    else
                    {
                        field_type.set_semantic(iter->second);
                    }
                }
            }
        }

        bool has_circular_struct_declarations(std::map<std::string, class_type_semantics> const& symbols, xlang_error_manager & error_manager)
        {
            if (contains_itself)
            {
                return true;
            }
            std::string symbol = this->get_fully_qualified_id();
            std::set<std::string> symbol_set{ symbol }; 
            if (has_circular_struct_declarations(symbols, symbol_set, error_manager))
            {
                contains_itself = true;
                error_manager.write_struct_field_error(get_decl_line(), std::string(symbol));
            }
            return contains_itself;
        }

        bool has_circular_struct_declarations(std::map<std::string, class_type_semantics> const& symbols, std::set<std::string> & symbol_set, xlang_error_manager & error_manager)
        {
            if (contains_itself)
            {
                return true;
            }
            for (auto const& field : m_fields)
            {
                auto field_type = field.first.get_semantic().get_resolved_target();
                if (std::holds_alternative<std::shared_ptr<struct_model>>(field_type))
                {
                    auto struct_field = std::get<std::shared_ptr<struct_model>>(field_type);
                    struct_field->resolve(symbols, error_manager);
                    if (!symbol_set.insert(struct_field->get_fully_qualified_id()).second
                        || struct_field->has_circular_struct_declarations(symbols, symbol_set, error_manager))
                    {
                        return true;
                    }
                    symbol_set.erase(struct_field->get_fully_qualified_id());
                }
            }
            return false;
        }

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
