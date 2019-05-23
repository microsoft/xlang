#pragma once

#include <string_view>

#include "base_model.h"
#include "method_model.h"
#include "model_types.h"
#include "compilation_unit.h"


namespace xlang::xmeta
{
    struct event_semantics
    {
        bool is_protected = false;
        bool is_static = false;
    };

    struct event_model : base_model
    {
        event_model() = delete;

        event_model(std::string_view const& id, 
                size_t decl_line, 
                std::string_view const& assembly_name, 
                event_semantics const& sem, 
                std::shared_ptr<method_model> const& add_method, 
                std::shared_ptr<method_model> const& remove_method,
                type_ref&& t) :
            base_model{ id, decl_line, assembly_name },
            m_semantic{ sem },
            m_add_method{ add_method },
            m_remove_method{ remove_method },
            m_type{ std::move(t) },
            m_implemented_event_ref{ "" }
        { }

        event_model(std::string_view const& id,
            size_t decl_line,
            std::string_view const& assembly_name,
            event_semantics const& sem,
            type_ref&& t) :
            base_model{ id, decl_line, assembly_name },
            m_semantic{ sem },
            m_type{ std::move(t) },
            m_implemented_event_ref{ "" }
        { }

        event_model(std::string_view const& id,
                size_t decl_line,
                std::string_view const& assembly_name,
                std::shared_ptr<method_model> const& add_method,
                std::shared_ptr<method_model> const& remove_method,
                type_ref&& t) :
            base_model{ id, decl_line, assembly_name },
            m_add_method{ add_method },
            m_remove_method{ remove_method },
            m_type{ std::move(t) },
            m_implemented_event_ref{ "" }
        { }


        auto const& get_add_method() const noexcept
        {
            return m_add_method;
        }

        auto const& get_remove_method() const noexcept
        {
            return m_remove_method;
        }

        auto const& get_semantic() const noexcept
        {
            return m_semantic;
        }

        auto const& get_type() const noexcept
        {
            return m_type;
        }

        void set_overridden_event_ref(std::shared_ptr<event_model> const& ref) noexcept
        {
            assert(ref != nullptr);
            m_implemented_event_ref.resolve(ref);
            m_add_method->set_overridden_method_ref(ref->get_add_method());
            m_remove_method->set_overridden_method_ref(ref->get_remove_method());
        }

        compilation_error set_add_method(std::shared_ptr<method_model> const& m)
        {
            if (!m)
            {
                // TODO: consider throwing an exception
                return compilation_error::passed;
            }
            if (m_add_method)
            {
                return compilation_error::accessor_exists;
            }
            m_add_method = m;
            return compilation_error::passed;
        }

        compilation_error set_remove_method(std::shared_ptr<method_model> const& m)
        {
            if (!m)
            {
                // TODO: consider throwing an exception
                return compilation_error::passed;
            }
            if (m_remove_method)
            {
                return compilation_error::accessor_exists;
            }
            m_remove_method = m;
            return compilation_error::passed;
        }

        void resolve(symbol_table & symbols, xlang_error_manager & error_manager, std::string const& fully_qualified_id)
        {
            assert(!m_type.get_semantic().is_resolved());
            std::string const& ref_name = m_type.get_semantic().get_ref_name();
            std::string symbol = ref_name.find(".") != std::string::npos
                ? ref_name : fully_qualified_id + "." + ref_name;
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

    private:
        event_semantics m_semantic;
        model_ref<std::shared_ptr<event_model>> m_implemented_event_ref;
        type_ref m_type;
        std::shared_ptr<method_model> m_add_method = nullptr;
        std::shared_ptr<method_model> m_remove_method = nullptr;
    };
}
