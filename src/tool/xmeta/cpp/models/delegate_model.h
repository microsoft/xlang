#pragma once

#include <string_view>
#include <optional>
#include <vector>

#include "base_model.h"
#include "model_types.h"
#include "formal_parameter_model.h"

namespace xlang::xmeta
{
    struct delegate_model : base_model
    {
        delegate_model(std::string_view const& id, size_t decl_line, std::optional<type_ref>&& return_type) :
            base_model{ id, decl_line },
            m_return_type{ std::move(return_type) }
        { }
        delegate_model() = delete;

        auto const& get_return_type() const noexcept
        {
            return m_return_type;
        }

        auto const& get_formal_parameters() const noexcept
        {
            return m_formal_parameters;
        }

        void add_formal_parameter(formal_parameter_model&& formal_param)
        {
            m_formal_parameters.emplace_back(std::move(formal_param));
        }

    private:
        std::optional<type_ref> m_return_type;
        std::vector<formal_parameter_model> m_formal_parameters;
    };
}
