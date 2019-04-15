#pragma once

#include <string_view>

#include "base_model.h"
#include "method_model.h"
#include "model_types.h"

namespace xlang::xmeta
{
    struct property_semantics
    {
        bool is_protected = false;
        bool is_static = false;
    };

    struct property_type_semantics
    {
        property_type_semantics() = delete;
        property_type_semantics(bool is_array, type_ref&& type_id) :
            m_is_array{ is_array },
            m_type_id{ std::move(type_id) }
        { }

    private:
        bool m_is_array;
        type_ref m_type_id;
    };

    struct property_model : base_model
    {
        property_model() = delete;
        property_model(std::string_view const& id, size_t decl_line, property_semantics const& sem, property_type_semantics&& type, std::shared_ptr<method_model> const& get_method, std::shared_ptr<method_model> const& set_method) :
            base_model{ id, decl_line },
            m_semantic{ sem },
            m_type{ std::move(type) },
            m_get_method{ get_method },
            m_set_method{ set_method }
        { }

        auto const& get_semantic() const noexcept
        {
            return m_semantic;
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

    private:
        property_semantics m_semantic;
        property_type_semantics m_type;
        std::shared_ptr<method_model> m_get_method;
        std::shared_ptr<method_model> m_set_method;
    };
}
