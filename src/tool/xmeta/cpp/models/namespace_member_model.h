#pragma once

#include <memory>
#include <optional>
#include <string_view>

#include "base_model.h"
#include "namespace_model.h"

namespace xlang::xmeta
{
    using class_type_semantics = std::variant<
        std::shared_ptr<class_model>,
        std::shared_ptr<enum_model>,
        std::shared_ptr<interface_model>,
        std::shared_ptr<struct_model>,
        std::shared_ptr<delegate_model>>;

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

        std::string const& get_fully_qualified_id() const noexcept
        {
            return std::string(m_containing_namespace_body->get_containing_namespace()->get_fully_qualified_id() + "." + get_id());
        }

    private:
        std::shared_ptr<namespace_body_model> m_containing_namespace_body;
    };
}
