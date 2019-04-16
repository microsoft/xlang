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
        struct_model() = delete;
        struct_model(std::string_view const& id, size_t decl_line, std::string_view const& assembly_name) :
            base_model{ id, decl_line, assembly_name }
        { }

        auto const& get_fields() const noexcept
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
