#pragma once
#include "formal_parameter_model.h"
#include "model_types.h"
#include "compilation_unit.h"

namespace xlang::xmeta
{
    // TODO: qualified_name will be a vector of strings once  we have using directives
    void formal_parameter_model::resolve(symbol_table & symbols, xlang_error_manager & error_manager, std::string const& qualified_name, method_association m_association)
    {
        if (!m_type.get_semantic().is_resolved())
        {
            std::string const& ref_name = m_type.get_semantic().get_ref_name();
            std::string symbol = ref_name.find(".") != std::string::npos ? ref_name : qualified_name + "." + ref_name;
            auto const& iter = symbols.get_symbol(symbol);
            if (std::holds_alternative<std::monostate>(iter))
            {
                if (m_association == method_association::None)
                {
                    error_manager.report_error(idl_error::UNRESOLVED_TYPE, get_decl_line(), symbol);
                }
            }
            else
            {
                m_type.set_semantic(iter);
            }
        }
    }
}
