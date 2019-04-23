#pragma once

#include <assert.h>
#include <string_view>
#include <vector>

#include "event_model.h"
#include "interface_model.h"
#include "method_model.h"
#include "model_ref.h"
#include "namespace_member_model.h"
#include "property_model.h"

namespace xlang::xmeta
{
    struct interface_model : namespace_member_model
    {
        interface_model() = delete;
        interface_model(std::string_view const& id, size_t decl_line, std::string_view const& assembly_name, std::shared_ptr<namespace_body_model> const& containing_ns_body) :
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

    private:
        std::vector<model_ref<std::shared_ptr<interface_model>>> m_interface_base_refs;
        // TODO: Add type parameters (generic types)

        // Members
        std::vector<std::shared_ptr<property_model>> m_properties;
        std::vector<std::shared_ptr<method_model>> m_methods;
        std::vector<std::shared_ptr<event_model>> m_events;
    };
}
