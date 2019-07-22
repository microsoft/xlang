#pragma once

#include "event_model.h"
#include "interface_model.h"
#include "method_model.h"
#include "model_ref.h"
#include "property_model.h"
#include "class_or_interface_model.h"
#include "xlang_error.h"

namespace xlang::xmeta
{
    std::shared_ptr<interface_model> create_interface_from(std::shared_ptr<xlang::meta::reader::TypeDef> type_def)
    {
        // TODO: this method may be required to import interfaces that are required by the runtime class. 
        return nullptr;
    }

    void interface_model::validate(xlang_error_manager & error_manager)
    {
        std::set<std::shared_ptr<interface_model>> bases = get_all_interface_bases();
        for (auto const& base : bases)
        {
            for (auto const& base_event : base->get_events())
            {
                if (member_exists(base_event->get_name()))
                {
                    error_manager.report_error(idl_error::DUPLICATE_TYPE_MEMBER_ID, get_decl_line(), base_event->get_name());
                    return;
                }
            }

            for (auto const& base_properties : base->get_properties())
            {
                if (member_exists(base_properties->get_name()))
                {
                    error_manager.report_error(idl_error::DUPLICATE_TYPE_MEMBER_ID, get_decl_line(), base_properties->get_name());
                    return;
                }
            }

            for (auto const& base_method : base->get_methods())
            {
                if (member_exists(base_method->get_name()))
                {
                    error_manager.report_error(idl_error::CANNOT_OVERLOAD_METHOD, base_method->get_decl_line(), base_method->get_name());
                    return;
                }
            }
        }
        class_or_interface_model::validate(error_manager);
    }

    void interface_model::resolve(symbol_table & symbols, xlang_error_manager & error_manager)
    {
        class_or_interface_model::resolve(symbols, error_manager);
    }

    bool interface_model::has_circular_inheritance(xlang_error_manager & error_manager)
    {
        if (contains_itself)
        {
            return true;
        }
        std::string symbol = this->get_qualified_name();
        std::set<std::string> symbol_set{ symbol };
        if (has_circular_inheritance(symbol_set, error_manager))
        {
            contains_itself = true;
            error_manager.report_error(idl_error::CIRCULAR_STRUCT_FIELD, get_decl_line(), symbol);
        }
        return contains_itself;
    }

    bool interface_model::has_circular_inheritance(std::set<std::string> & symbol_set, xlang_error_manager & error_manager)
    {
        if (contains_itself)
        {
            return true;
        }
        for (auto const& base : this->get_interface_bases())
        {
            if (!base.get_semantic().is_resolved())
            {
                return false;
            }
            auto const& type = base.get_semantic().get_resolved_target();
            if (std::holds_alternative<std::shared_ptr<interface_model>>(type))
            {
                std::shared_ptr<interface_model> interface_base = std::get<std::shared_ptr<interface_model>>(type);
                if (!symbol_set.insert(interface_base->get_qualified_name()).second
                    || interface_base->has_circular_inheritance(symbol_set, error_manager))
                {
                    return true;
                }
                symbol_set.erase(interface_base->get_qualified_name());
            }
        }
        return false;
    }
}