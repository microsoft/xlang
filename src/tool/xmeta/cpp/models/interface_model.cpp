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
        if (!type_def->is_interface())
        {
            xlang::throw_invalid("Type def is not an interface");
        }
        auto model = std::make_shared<interface_model>(type_def->TypeName(), 0, type_def->get_database().Assembly[0].Name(), type_def->TypeNamespace());

        for (auto const& methods : type_def->MethodList())
        {

        }
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
                    error_manager.write_type_member_exists_error(get_decl_line(), base_event->get_name(), get_qualified_name());
                    return;
                }
            }

            for (auto const& base_properties : base->get_properties())
            {
                if (member_exists(base_properties->get_name()))
                {
                    error_manager.write_type_member_exists_error(get_decl_line(), base_properties->get_name(), get_qualified_name());
                    return;
                }
            }

            for (auto const& base_method : base->get_methods())
            {
                if (member_exists(base_method->get_name()))
                {
                    error_manager.write_type_member_exists_error(get_decl_line(), base_method->get_name(), get_qualified_name());
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
            error_manager.write_struct_field_error(get_decl_line(), std::string(symbol));
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