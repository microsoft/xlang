#pragma once

#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "model_types.h"
#include "namespace_member_model.h"
#include "property_model.h"

namespace xlang::xmeta
{
    struct struct_model : namespace_member_model
    {
        struct_model() = delete;
        struct_model(std::string_view const& id, size_t decl_line, std::string_view const& assembly_name, std::shared_ptr<namespace_body_model> const& containing_ns_body) :
            namespace_member_model{ id, decl_line, assembly_name, containing_ns_body }
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
