#include "event_model.h"
#include "method_model.h"
#include "model_types.h"
#include "compilation_unit.h"

namespace xlang::xmeta
{
    void event_model::set_overridden_event_ref(std::shared_ptr<event_model> const& ref) noexcept
    {
        assert(ref != nullptr);
        m_implemented_event_ref.resolve(ref);
        m_add_method->set_overridden_method_ref(ref->get_add_method());
        m_remove_method->set_overridden_method_ref(ref->get_remove_method());
    }

    semantic_error event_model::set_add_method(std::shared_ptr<method_model> const& m)
    {
        if (!m)
        {
            // TODO: consider throwing an exception
            return semantic_error::passed;
        }
        if (m_add_method)
        {
            return semantic_error::accessor_exists;
        }
        m_add_method = m;
        return semantic_error::passed;
    }

    semantic_error event_model::set_remove_method(std::shared_ptr<method_model> const& m)
    {
        if (!m)
        {
            // TODO: consider throwing an exception
            return semantic_error::passed;
        }
        if (m_remove_method)
        {
            return semantic_error::accessor_exists;
        }
        m_remove_method = m;
        return semantic_error::passed;
    }

    void event_model::resolve(symbol_table & symbols, xlang_error_manager & error_manager, std::string const& qualified_name)
    {
        assert(!m_type.get_semantic().is_resolved());
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
            if (std::holds_alternative<std::shared_ptr<delegate_model>>(iter))
            {
                m_type.set_semantic(iter);
            }
            else
            {
                error_manager.write_not_a_delegate_error(get_decl_line(), symbol);
            }
        }
    }
}