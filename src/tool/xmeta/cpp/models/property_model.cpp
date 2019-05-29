#pragma once
#include <string_view>
#include "property_model.h"

namespace xlang::xmeta
{
    void property_model::set_overridden_property_ref(std::shared_ptr<property_model> const& ref) noexcept
    {
        assert(ref != nullptr);
        m_implemented_property_ref.resolve(ref);
        if (ref->get_get_method())
        {
            m_get_method->set_overridden_method_ref(ref->get_get_method());
        }
        if (ref->get_set_method())
        {
            m_set_method->set_overridden_method_ref(ref->get_set_method());
        }
    }

    compilation_error property_model::set_get_method(std::shared_ptr<method_model> const& m)
    {
        if (!m)
        {
            // TODO: consider throwing an exception
            return compilation_error::passed;
        }
        if (m_get_method)
        {
            return compilation_error::accessor_exists;
        }
        m_get_method = m;
        return compilation_error::passed;
    }

    compilation_error property_model::set_set_method(std::shared_ptr<method_model> const& m)
    {
        if (!m)
        {
            // TODO: consider throwing an exception
            return compilation_error::passed;
        }
        if (m_set_method)
        {
            return compilation_error::accessor_exists;
        }
        m_set_method = m;
        return compilation_error::passed;
    }

    void property_model::validate(xlang_error_manager & error_manager)
    {
        if (!m_get_method)
        {
            error_manager.write_property_accessor_error(get_decl_line(), get_name());
        }
    }

    void property_model::resolve(symbol_table & symbols, xlang_error_manager & error_manager, std::string const& qualified_name)
    {
        if (!m_type.get_semantic().is_resolved())
        {
            /* Events should not have been resolved. If it was, it means it was not a
            class type and not a delegate type */
            std::string const& ref_name = m_type.get_semantic().get_ref_name();
            std::string symbol = ref_name.find(".") != std::string::npos
                ? ref_name : qualified_name + "." + ref_name;
            auto const& iter = symbols.get_symbol(symbol);
            if (std::holds_alternative<std::monostate>(iter))
            {
                error_manager.write_unresolved_type_error(get_decl_line(), symbol);
            }
            else
            {
                m_type.set_semantic(iter);
            }
        }
    }
}