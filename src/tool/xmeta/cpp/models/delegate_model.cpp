#include "delegate_model.h"
#include "model_types.h"
#include "xlang_error.h"
#include "compilation_unit.h"
#include "namespace_member_model.h"
#include "formal_parameter_model.h"

namespace xlang::xmeta
{
    void delegate_model::resolve(symbol_table & symbols, xlang_error_manager & error_manager)
    {
        if (m_return_type)
        {
            if (!m_return_type->get_semantic().is_resolved())
            {
                // TODO: Once we have using directives, we will need to go through many fully_qualified_ids here
                std::string const& ref_name = m_return_type->get_semantic().get_ref_name();
                std::string symbol = ref_name.find(".") != std::string::npos ? ref_name : this->get_containing_namespace_body()->get_containing_namespace()->get_qualified_name() + "." + ref_name;
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
            param.resolve(symbols, error_manager, this->get_containing_namespace_body()->get_containing_namespace()->get_qualified_name());
        }
    }

    void delegate_model::add_formal_parameter(formal_parameter_model&& formal_param)
    {
        m_formal_parameters.emplace_back(std::move(formal_param));
    }

}