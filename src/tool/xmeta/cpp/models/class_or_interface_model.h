#pragma once

#include <memory>
#include <string_view>

#include "namespace_member_model.h"
#include "property_model.h"

namespace xlang::xmeta
{
    struct class_or_interface_model : namespace_member_model
    {
        class_or_interface_model() = delete;
        class_or_interface_model(std::string_view const& id, size_t decl_line, std::string_view const& assembly_name, std::shared_ptr<namespace_body_model> const& containing_ns_body) :
            namespace_member_model{ id, decl_line, assembly_name, containing_ns_body }
        { }

        auto const& get_interface_bases() const noexcept
        {
            return m_interface_base_refs;
        }

        auto const& get_properties() const noexcept
        {
            return m_properties;
        }

        auto const& get_methods() const noexcept
        {
            return m_methods;
        }

        auto const& get_events() const noexcept
        {
            return m_events;
        }

        void add_interface_base_ref(std::string_view const& interface_base_ref)
        {
            m_interface_base_refs.emplace_back(interface_base_ref);
        }

        void add_member(std::shared_ptr<property_model> const& member)
        {
            m_properties.emplace_back(member);
        }

        void add_member(std::shared_ptr<method_model> const& member)
        {
            m_methods.emplace_back(member);
        }

        void add_member(std::shared_ptr<event_model> const& member)
        {
            m_events.emplace_back(member);
        }

        bool member_id_exists(std::string_view const& id)
        {
            return contains_id(m_properties, id) ||
                contains_id(m_methods, id) ||
                contains_id(m_events, id);
        }

        void resolve(std::map<std::string, class_type_semantics> symbols)
        {
            for (auto & m_method : m_methods)
            {
                m_method->resolve(symbols, this->get_containing_namespace_body()->get_containing_namespace()->get_fully_qualified_id());
            }
            for (auto & m_property : m_properties)
            {
                m_property->resolve(symbols, this->get_containing_namespace_body()->get_containing_namespace()->get_fully_qualified_id());
            }
            for (auto & m_event : m_events)
            {
                m_event->resolve(symbols, this->get_containing_namespace_body()->get_containing_namespace()->get_fully_qualified_id());
            }

            for (auto & interface_base : m_interface_base_refs)
            {
                if (interface_base.get_semantic().is_resolved())
                {
                    /* Events should not have been resolved. If it was, it means it was not a
                    class type and not a delegate type */
                    return;
                }
                std::string ref_name = interface_base.get_semantic().get_ref_name();
                std::string symbol = ref_name.find(".") != std::string::npos
                    ? ref_name : this->get_containing_namespace_body()->get_containing_namespace()->get_fully_qualified_id() + "." + ref_name;
                auto iter = symbols.find(symbol);
                if (iter == symbols.end())
                {
                    // TODO: Record the unresolved type and continue once we have a good error story for reporting errors in models
                }
                else
                {
                    if (std::holds_alternative<std::shared_ptr<interface_model>>(iter->second))
                    {
                        interface_base.set_semantic(iter->second);
                    }
                    else
                    {
                        // TODO: Error report the non-interface type
                    }
                }
            }

        }

    private:
        std::vector<type_ref> m_interface_base_refs;
        // TODO: Add type parameters (generic types)

        // Members
        std::vector<std::shared_ptr<property_model>> m_properties;
        std::vector<std::shared_ptr<method_model>> m_methods;
        std::vector<std::shared_ptr<event_model>> m_events;
    };
}
