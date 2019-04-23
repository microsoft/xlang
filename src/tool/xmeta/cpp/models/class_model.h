#pragma once

#include <memory>
#include <string_view>
#include <string>
#include <vector>
#include <variant>

#include "event_model.h"
#include "interface_model.h"
#include "method_model.h"
#include "model_ref.h"
#include "namespace_member_model.h"
#include "property_model.h"

namespace xlang::xmeta
{
    // Note: All classes are implicitly public, and explicitly specifying this is not allowed.
    enum class class_semantics
    {
        sealed_instance_class,
        unsealed_class,
        static_class
    };

    struct class_model : namespace_member_model
    {
        class_model() = delete;
        class_model(std::string_view const& id, size_t decl_line, std::string_view const& assembly_name, std::shared_ptr<namespace_body_model> const& containing_ns_body, class_semantics const& sem, std::string_view const& base_id) :
            namespace_member_model{ id, decl_line, assembly_name, containing_ns_body },
            m_semantic{ sem },
            m_class_base_ref{ base_id }
        { }

        auto const& get_class_base_ref() const noexcept
        {
            return m_class_base_ref;
        }

        auto const& get_interface_base_refs() const noexcept
        {
            return m_interface_base_refs;
        }

        auto const& get_semantic() const noexcept
        {
            return m_semantic;
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

        void add_interface_base_ref(size_t index, std::shared_ptr<interface_model> const& interface_base_ref)
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

        void add_protected_interface_ref(std::shared_ptr<interface_model> const& protected_interface_ref)
        {
            m_static_interfaces.emplace_back(protected_interface_ref);
        }

        void add_static_interface_ref(std::shared_ptr<interface_model> const& static_interface_ref)
        {
            m_static_interfaces.emplace_back(static_interface_ref);
        }

    private:
        model_ref<std::shared_ptr<class_model>> m_class_base_ref;
        std::vector<model_ref<std::shared_ptr<interface_model>>> m_interface_base_refs;
        class_semantics m_semantic;
        // TODO: Add type parameters (generic types)

        // Members
        std::vector<std::shared_ptr<property_model>> m_properties;
        std::vector<std::shared_ptr<method_model>> m_methods;
        std::vector<std::shared_ptr<event_model>> m_events;

        std::vector<std::shared_ptr<interface_model>> m_static_interfaces;
        std::vector<std::shared_ptr<interface_model>> m_protected_interfaces;
    };
}
