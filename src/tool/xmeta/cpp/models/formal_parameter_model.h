#pragma once

#include <string>
#include <string_view>

#include "base_model.h"
#include "model_types.h"

namespace xlang::xmeta
{
    enum class parameter_semantics
    {
        in,
        ref,
        const_ref,
        out
    };

    struct formal_parameter_model : base_model
    {
        formal_parameter_model(std::string_view const& id, size_t decl_line, parameter_semantics sem, type_ref&& type) :
            base_model{ id, decl_line },
            m_semantic{ sem },
            m_type{ std::move(type) }
        { }

        auto const& get_semantic() const noexcept
        {
            return m_semantic;
        }

        auto const& get_type() const noexcept
        {
            return m_type;
        }

    private:
        parameter_semantics m_semantic;
        type_ref m_type;
    };
}
