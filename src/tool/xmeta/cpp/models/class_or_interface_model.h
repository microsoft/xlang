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
            m_interface_base_refs.emplace_back(std::string(interface_base_ref));
        }

        void add_interface_base_ref(size_t index, std::shared_ptr<interface_model> interface_base_ref)
        {
            assert(index < m_interface_base_refs.size());
            m_interface_base_refs[index].resolve(interface_base_ref);
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
        }

    private:
        std::vector<model_ref<std::shared_ptr<interface_model>>> m_interface_base_refs;
        // TODO: Add type parameters (generic types)

        // Members
        std::vector<std::shared_ptr<property_model>> m_properties;
        std::vector<std::shared_ptr<method_model>> m_methods;
        std::vector<std::shared_ptr<event_model>> m_events;
    };
}
