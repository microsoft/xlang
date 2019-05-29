#pragma once

#include <string_view>
#include <optional>
#include <vector>

#include "base_model.h"
#include "namespace_member_model.h"
#include "formal_parameter_model.h"

namespace xlang::xmeta
{
    struct delegate_model : namespace_member_model
    {
        delegate_model() = delete;
        delegate_model(std::string_view const& id, size_t decl_line, std::string_view const& assembly_name, std::shared_ptr<namespace_body_model> const& containing_ns_body, std::optional<type_ref>&& return_type) :
            namespace_member_model{ id, decl_line, assembly_name, containing_ns_body },
            m_return_type{ std::move(return_type) }
        { }

        delegate_model(std::string_view const& id, size_t decl_line, std::string_view const& assembly_name, std::string_view const& containing_ns_name, std::optional<type_ref>&& return_type) :
            namespace_member_model{ id, decl_line, assembly_name, containing_ns_name },
            m_return_type{ std::move(return_type) }
        { }

        auto const& get_return_type() const noexcept
        {
            return m_return_type;
        }

        auto const& get_formal_parameters() const noexcept
        {
            return m_formal_parameters;
        }

        void resolve(symbol_table & symbols, xlang_error_manager & error_manager);

        void add_formal_parameter(formal_parameter_model&& formal_param);

    private:
        std::optional<type_ref> m_return_type;
        std::vector<formal_parameter_model> m_formal_parameters;
    };
}
