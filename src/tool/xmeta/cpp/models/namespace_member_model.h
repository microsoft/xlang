#pragma once

#include <memory>
#include <optional>
#include <string_view>

#include "base_model.h"
#include "namespace_model.h"

namespace xlang::xmeta
{
    struct namespace_member_model : base_model
    {
        namespace_member_model() = delete;

        namespace_member_model(std::string_view const& id, 
                size_t decl_line, 
                std::string_view const& assembly_name, 
                std::shared_ptr<namespace_body_model> const& containing_ns_body) :
            base_model{ id, decl_line, assembly_name },
            m_containing_namespace_body{ containing_ns_body }
        { }

        namespace_member_model(std::string_view const& id, 
                size_t decl_line, 
                std::string_view const& assembly_name, 
                std::string_view const& containing_namespace_name) :
            base_model{ id, decl_line, assembly_name },
            m_containing_namespace_body{ nullptr },
            containing_namespace_name{ containing_namespace_name }
        { }

        auto const& get_containing_namespace_body() const noexcept
        {
            return m_containing_namespace_body;
        }

        std::string const get_fully_qualified_id() const noexcept;

    private:
        std::shared_ptr<namespace_body_model> m_containing_namespace_body;
        std::optional<std::string> containing_namespace_name;
    };
}
