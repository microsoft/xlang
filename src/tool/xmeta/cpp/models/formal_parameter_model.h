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

    //TODO: Reevaluate the name formal_parameter_model. Possibly change to just parameter_model. 
    struct formal_parameter_model : base_model
    {
        formal_parameter_model() = delete;
        formal_parameter_model(std::string_view const& name, size_t decl_line, std::string_view const& assembly_name, parameter_semantics sem, type_ref&& type) :
            base_model{ name, decl_line, assembly_name },
            m_semantic{ sem },
            m_type{ std::move(type) }
        { }

        formal_parameter_model(std::string_view const& name, parameter_semantics sem, type_ref&& type) :
            base_model{ name, 0, "test_only" },
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

        // TODO: qualified_name will be a vector of strings once  we have using directives
        void resolve(symbol_table & symbols, xlang_error_manager & error_manager, std::string const& qualified_name, method_association m_association = method_association::None);

    private:
        parameter_semantics m_semantic;
        type_ref m_type;
    };
}
