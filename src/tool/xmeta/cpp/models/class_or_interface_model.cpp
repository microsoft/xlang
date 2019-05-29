#pragma once

#include "xmeta_models.h"

namespace xlang::xmeta
{
    void class_or_interface_model::add_interface_base_ref(std::string_view const& interface_base_ref)
    {
        m_interface_base_refs.emplace_back(interface_base_ref);
    }

    semantic_error class_or_interface_model::add_member(std::shared_ptr<property_model> const& member)
    {
        if (member_exists(member->get_name()))
        {
            return semantic_error::symbol_exists;
        }
        m_properties.emplace_back(member);
        return semantic_error::passed;
    }

    semantic_error class_or_interface_model::add_member(std::shared_ptr<method_model> const& member)
    {
        if (event_or_property_exists(member->get_name()))
        {
            return semantic_error::symbol_exists;
        }

        for (auto const& overloading_method : m_methods)
        {
            if (overloading_method->get_name() == member->get_name())
            {
                if (overloading_method->get_formal_parameters().size() == member->get_formal_parameters().size())
                {
                    return semantic_error::symbol_exists;
                }
                if ((overloading_method->get_return_type() == std::nullopt && member->get_return_type() != std::nullopt)
                        || (overloading_method->get_return_type() != std::nullopt && member->get_return_type() == std::nullopt))
                {
                    return semantic_error::symbol_exists;
                }
                if (overloading_method->get_return_type() != std::nullopt && member->get_return_type() != std::nullopt)
                {
                    if (!(*overloading_method->get_return_type() == *member->get_return_type()))
                    {
                        return semantic_error::symbol_exists;
                    }
                }
            }
        }
        m_methods.emplace_back(member);
        return semantic_error::passed;
    }

    semantic_error class_or_interface_model::add_member(std::shared_ptr<event_model> const& member)
    {
        if (member_exists(member->get_name()))
        {
            return semantic_error::symbol_exists;
        }
        m_events.emplace_back(member);
        return semantic_error::passed;
    }

    bool class_or_interface_model::member_exists(std::string_view const& name)
    {
        return contains_id(m_properties, name) ||
            contains_id(m_methods, name) ||
            contains_id(m_events, name);
    }

    bool class_or_interface_model::event_or_property_exists(std::string_view const& name)
    {
        return contains_id(m_properties, name) || contains_id(m_events, name);
    }

    bool class_or_interface_model::property_exists(std::string_view const& name)
    {
        return contains_id(m_properties, name);
    }

    bool class_or_interface_model::method_exists(std::string_view const& name)
    {
        return contains_id(m_methods, name);
    }

    std::shared_ptr<property_model> const& class_or_interface_model::get_property_member(std::string const& member_id)
    {
        return *get_by_name(m_properties, member_id);
    }

    std::shared_ptr<method_model> const& class_or_interface_model::get_method_member(std::string const& member_id)
    {
        return *get_by_name(m_methods, member_id);
    }

    void class_or_interface_model::validate(xlang_error_manager & error_manager)
    {
        for (auto const& prop : m_properties)
        {
            prop->validate(error_manager);
        }
    }

    void class_or_interface_model::resolve(symbol_table & symbols, xlang_error_manager & error_manager)
    {
        for (auto & m_method : m_methods)
        {
            m_method->resolve(symbols, error_manager, this->get_containing_namespace_body()->get_containing_namespace()->get_qualified_name());
        }
        for (auto & m_property : m_properties)
        {
            m_property->resolve(symbols, error_manager, this->get_containing_namespace_body()->get_containing_namespace()->get_qualified_name());
        }
        for (auto & m_event : m_events)
        {
            m_event->resolve(symbols, error_manager, this->get_containing_namespace_body()->get_containing_namespace()->get_qualified_name());
        }

        for (auto & interface_base : m_interface_base_refs)
        {
            assert(!interface_base.get_semantic().is_resolved());
            std::string ref_name = interface_base.get_semantic().get_ref_name();
            std::string symbol = ref_name.find(".") != std::string::npos
                ? ref_name : this->get_containing_namespace_body()->get_containing_namespace()->get_qualified_name() + "." + ref_name;

            auto iter = symbols.get_symbol(symbol);
            if (std::holds_alternative<std::monostate>(iter))
            {
                error_manager.write_unresolved_type_error(get_decl_line(), symbol);
            }
            else
            {
                if (std::holds_alternative<std::shared_ptr<interface_model>>(iter))
                {
                    interface_base.set_semantic(iter);
                }
                else
                {
                    error_manager.write_not_an_interface_error(get_decl_line(), symbol);
                }
            }
        }
    }

    std::set<std::shared_ptr<interface_model>> class_or_interface_model::get_all_interface_bases()
    {
        std::set<std::shared_ptr<interface_model>> bases;
        for (auto const& base : this->get_interface_bases())
        {
            auto const& type = base.get_semantic().get_resolved_target();
            assert(base.get_semantic().is_resolved());
            assert(std::holds_alternative<std::shared_ptr<interface_model>>(type));
            std::shared_ptr<interface_model> const& interface_base = std::get<std::shared_ptr<interface_model>>(base.get_semantic().get_resolved_target());
            bases.insert(interface_base);
            std::set<std::shared_ptr<interface_model>> super_bases = interface_base->get_all_interface_bases();
            for (auto const& iter : super_bases)
            {
                bases.insert(iter);
            }
        }
        return bases;
    }
}