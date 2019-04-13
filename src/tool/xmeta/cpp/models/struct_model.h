#pragma once

#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "base_model.h"
#include "model_types.h"
#include "property_model.h"

namespace xlang::xmeta
{
    struct struct_model : base_model
    {
        struct_model(std::string_view const& id, size_t decl_line) : base_model{ id, decl_line } { }
        struct_model() = delete;

        auto const& get_fields() const
        {
            return m_fields;
        }

        void add_field(std::pair<type_ref, std::string>&& field)
        {
            m_fields.emplace_back(std::move(field));
        }

    private:
        std::vector<std::pair<type_ref, std::string>> m_fields;
    };
}
