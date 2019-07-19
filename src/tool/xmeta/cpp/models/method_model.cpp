#pragma once

#include "method_model.h"
#include "formal_parameter_model.h"
#include "model_ref.h"
#include "model_types.h"
#include "meta_reader.h"

namespace xlang::xmeta
{
    using namespace xlang::meta::reader;

    std::shared_ptr<method_model> create_method_from(std::shared_ptr<MethodDef> method_def)
    {
        auto const& return_type = method_def->Signature().ReturnType().Type().Type();
        if (std::holds_alternative<ElementType>(return_type))
        {
            auto const& return_elemment_type = std::get<ElementType>(return_type);
        }

        auto model = std::make_shared<method_model>(method_def->Name(), 0, method_def->get_database().Assembly[0].Name(), std::move(std::nullopt), method_association::None);
        return model;
    }

    void method_model::set_overridden_method_ref(std::shared_ptr<method_model> const& ref) noexcept
    {
        assert(ref != nullptr);
        m_implemented_method_ref.resolve(ref);
    }

    void method_model::resolve(symbol_table & symbols, xlang_error_manager & error_manager, std::string const& qualified_name)
    {
        if (m_return_type)
        {
            if (!m_return_type->get_semantic().is_resolved())
            {
                // TODO: Once we have using directives, we will need to go through many fully_qualified_ids here
                std::string const& ref_name = m_return_type->get_semantic().get_ref_name();
                std::string symbol = ref_name.find(".") != std::string::npos ? ref_name : qualified_name + "." + ref_name;
                auto const& iter = symbols.get_symbol(symbol);
                if (std::holds_alternative<std::monostate>(iter))
                {
                    if (m_association == method_association::None) // this check is in place so we don't report  the error twice
                    {
                        error_manager.report_error(idl_error::UNRESOLVED_TYPE, get_decl_line(), symbol);
                    }
                }
                else
                {
                    m_return_type->set_semantic(iter);
                }
            }
        }
        for (formal_parameter_model & param : m_formal_parameters)
        {
            param.resolve(symbols, error_manager, qualified_name, m_association);
        }
    }
}