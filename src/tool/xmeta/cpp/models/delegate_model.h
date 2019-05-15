#pragma once

#include <string_view>
#include <optional>
#include <vector>

#include "base_model.h"
#include "model_types.h"
#include "xlang_error.h"
#include "compilation_unit.h"

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

        void resolve(symbol_table & symbols, xlang_error_manager & error_manager)
        {
            if (m_return_type)
            {
                if (!m_return_type->get_semantic().is_resolved())
                {
                    // TODO: Once we have using directives, we will need to go through many fully_qualified_ids here
                    std::string const& ref_name = m_return_type->get_semantic().get_ref_name();
                    std::string symbol = ref_name.find(".") != std::string::npos ? ref_name : this->get_containing_namespace_body()->get_containing_namespace()->get_fully_qualified_id() + "." + ref_name;
                    auto const& iter = symbols.get_symbol(symbol);
                    if (std::holds_alternative<std::monostate>(iter))
                    {
                        error_manager.write_unresolved_type_error(get_decl_line(), symbol);
                    }
                    else
                    {
                        m_return_type->set_semantic(iter);
                    }
                }
            }
            for (formal_parameter_model & param : m_formal_parameters)
            {
                param.resolve(symbols, error_manager, this->get_containing_namespace_body()->get_containing_namespace()->get_fully_qualified_id());
            }
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
