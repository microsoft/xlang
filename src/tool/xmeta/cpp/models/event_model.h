#pragma once

#include <string_view>

#include "base_model.h"
#include "method_model.h"
#include "model_types.h"

namespace xlang::xmeta
{
    struct event_semantics
    {
        bool is_protected = false;
        bool is_static = false;
    };

    struct event_model : base_model
    {
        event_model(std::string_view const& id, size_t decl_line, event_semantics const& sem, std::shared_ptr<method_model> const& add_method_ref, std::shared_ptr<method_model> const& remove_method_ref, type_ref&& t) :
            base_model{ id, decl_line },
            m_semantic{ sem },
            m_add_method_ref{ add_method_ref },
            m_remove_method_ref{ remove_method_ref },
            m_type{ std::move(t) }
        { }
        event_model() = delete;

        auto const& get_add_method_ref() const noexcept
        {
            return m_add_method_ref;
        }

        auto const& get_remove_method_ref() const noexcept
        {
            return m_remove_method_ref;
        }

        auto const& get_semantic() const noexcept
        {
            return m_semantic;
        }

        auto const& get_type() const noexcept
        {
            return m_type;
        }

    private:
        event_semantics m_semantic;
        type_ref m_type;

        std::shared_ptr<method_model> m_add_method_ref;
        std::shared_ptr<method_model> m_remove_method_ref;
    };
}
