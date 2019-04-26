#pragma once

#include <optional>
#include <string_view>

#include "base_model.h"
#include "method_model.h"
#include "model_types.h"

namespace xlang::xmeta
{
    struct property_semantics
    {
        bool is_static;
        bool is_protected;
    };

    struct property_model : base_model
    {
        property_model() = delete;
        property_model(
            std::string_view const& id,
            size_t decl_line,
            std::string_view const& assembly_name,
            type_ref&& type,
            bool is_array,
            property_semantics&& sem,
            bool get_declared,
            bool set_declared,
            bool get_declared_first = true) :
                base_model{ id, decl_line, assembly_name },
                m_type{ std::move(type) },
                m_is_array{ is_array },
                m_semantic{std::move(sem)},
                m_get_declared_first{ get_declared_first }
        {
            if (get_declared)
            {
                create_get_method();
            }
            if (set_declared)
            {
                create_set_method();
            }
        }

        auto is_array() const noexcept
        {
            return m_is_array;
        }

        auto is_get_method_declared_first() const noexcept
        {
            return m_get_declared_first;
        }

        auto const& get_type() const noexcept
        {
            return m_type;
        }

        auto const& get_get_method() const noexcept
        {
            return m_get_method;
        }

        auto const& get_set_method() const noexcept
        {
            return m_set_method;
        }

        auto const& get_semantic() const noexcept
        {
            return m_semantic;
        }

        void create_get_method()
        {
            m_get_method = std::make_shared<method_model>(
                "get_" + get_id(),
                get_decl_line(),
                get_assembly_name(),
                type_ref{ m_type },
                std::vector<formal_parameter_model>{},
                method_semantics{});
        }

        void create_set_method()
        {
            m_set_method = std::make_shared<method_model>(
                "set_" + get_id(),
                get_decl_line(),
                get_assembly_name(),
                type_ref{ m_type },
                std::vector<formal_parameter_model>{},
                method_semantics{});
        }

    private:
        type_ref m_type;
        bool m_is_array;
        property_semantics m_semantic;
        bool m_get_declared_first;
        std::shared_ptr<method_model> m_get_method;
        std::shared_ptr<method_model> m_set_method;
    };
}
