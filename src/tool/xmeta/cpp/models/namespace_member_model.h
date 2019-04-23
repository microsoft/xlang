#pragma once

#include <memory>
#include <string_view>

#include "base_model.h"
#include "namespace_model.h"

namespace xlang::xmeta
{
    struct namespace_member_model : base_model
    {
        namespace_member_model() = delete;
        namespace_member_model(std::string_view const& id, size_t decl_line, std::string_view const& assembly_name, std::shared_ptr<namespace_body_model> const& containing_ns_body) :
            base_model{ id, decl_line, assembly_name },
            m_containing_namespace_body{ containing_ns_body }
        { }

        auto const& get_containing_namespace_body() const noexcept
        {
            return m_containing_namespace_body;
        }

        auto const& get_fully_qualified_name()
        {
            auto const& ns_name = m_containing_namespace_body->get_containing_namespace()->get_full_name();
            return (ns_name == "")
                ? get_id()
                : ns_name + "." + get_id();
        }

        auto const& get_fully_qualified_member_name(std::shared_ptr<base_model> const& member)
        {
            auto const& ns_name = m_containing_namespace_body->get_containing_namespace()->get_full_name();
            return (ns_name == "")
                ? member->get_id()
                : ns_name + "." + member->get_id();
        }

    private:
        std::shared_ptr<namespace_body_model> m_containing_namespace_body;
    };
}
