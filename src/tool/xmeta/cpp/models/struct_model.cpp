#pragma once

#include "model_types.h"
#include "namespace_member_model.h"
#include "property_model.h"
#include "struct_model.h"

namespace xlang::xmeta
{
    void struct_model::resolve(symbol_table & symbols, xlang_error_manager & error_manager)
    {
        for (auto & field : m_fields)
        {
            type_ref & field_type = field.first;
            if (!field_type.get_semantic().is_resolved())
            {
                // TODO: Once we have using directives, we will need to go through many fully_qualified_ids here
                std::string const& ref_name = field_type.get_semantic().get_ref_name();
                std::string symbol = ref_name.find(".") != std::string::npos
                    ? ref_name : this->get_containing_namespace_body()->get_containing_namespace()->get_qualified_name() + "." + ref_name;
                auto const& iter = symbols.get_symbol(symbol);
                if (std::holds_alternative<std::monostate>(iter))
                {
                    error_manager.report_error(idl_error::UNRESOLVED_TYPE, get_decl_line(), symbol);
                }
                else
                {
                    field_type.set_semantic(iter);
                }
            }
        }
    }

    bool struct_model::has_circular_struct_declarations(xlang_error_manager & error_manager)
    {
        if (contains_itself)
        {
            return true;
        }
        std::string symbol = this->get_qualified_name();
        std::set<std::string> symbol_set{ symbol };
        if (has_circular_struct_declarations(symbol_set, error_manager))
        {
            contains_itself = true;
            error_manager.report_error(idl_error::CIRCULAR_STRUCT_FIELD, get_decl_line(), symbol);
        }
        return contains_itself;
    }

    bool struct_model::has_circular_struct_declarations(std::set<std::string> & symbol_set, xlang_error_manager & error_manager)
    {
        if (contains_itself)
        {
            return true;
        }
        for (auto const& field : m_fields)
        {
            auto const& field_type = field.first.get_semantic().get_resolved_target();
            if (std::holds_alternative<std::shared_ptr<struct_model>>(field_type))
            {
                auto const& struct_field = std::get<std::shared_ptr<struct_model>>(field_type);
                if (!symbol_set.insert(struct_field->get_qualified_name()).second
                    || struct_field->has_circular_struct_declarations(symbol_set, error_manager))
                {
                    return true;
                }
                symbol_set.erase(struct_field->get_qualified_name());
            }
        }
        return false;
    }
}