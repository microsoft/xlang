#pragma once

#include <memory>
#include <string_view>
#include <string>
#include <vector>
#include <variant>

#include "base_model.h"
#include "interface_model.h"
#include "method_model.h"
#include "property_model.h"
#include "event_model.h"

namespace xlang::xmeta
{
    // Note: All classes are implicitly public, and explicitly specifying this is not allowed.
    enum class class_semantics
    {
        sealed_instance_class,
        unsealed_class,
        static_class
    };

    struct class_model : base_model
    {
        class_model(std::string_view const& id, size_t decl_line, class_semantics const& sem, std::string_view const& base_id) :
            base_model{ id, decl_line },
            m_semantic{ sem },
            m_class_base_ref{ std::string(base_id) }
        { }
        class_model() = delete;

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

        auto const& get_members() const noexcept
        {
            return m_members;
        }

        void add_interface_base_ref(std::string_view const& interface_base_ref)
        {
            m_interface_base_refs.emplace_back(std::string(interface_base_ref));
        }

        void add_interface_base_ref(size_t index, std::shared_ptr<interface_model> const& interface_base_ref)
        {
            assert(index < m_interface_base_refs.size());
            m_interface_base_refs[index] = interface_base_ref;
        }

        void add_member(std::shared_ptr<property_model> const& member)
        {
            m_members.emplace_back(member);
        }

        void add_member(std::shared_ptr<method_model> const& member)
        {
            m_members.emplace_back(member);
        }

        void add_member(std::shared_ptr<event_model> const& member)
        {
            m_members.emplace_back(member);
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
        std::variant<std::string, std::shared_ptr<class_model>> m_class_base_ref;
        std::vector<std::variant<std::string, std::shared_ptr<interface_model>>> m_interface_base_refs;
        class_semantics m_semantic;
        // TODO: Add type parameters (generic types)

        std::vector<std::variant<
            std::shared_ptr<property_model>,
            std::shared_ptr<method_model>,
            std::shared_ptr<event_model>>> m_members;

        std::vector<std::shared_ptr<interface_model>> m_static_interfaces;
        std::vector<std::shared_ptr<interface_model>> m_protected_interfaces;
    };
}
