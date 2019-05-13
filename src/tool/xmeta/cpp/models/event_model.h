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
            m_type{ std::move(t) }
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
            m_type{ std::move(t) }
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

        void resolve(symbol_table & symbols, xlang_error_manager & error_manager, std::string fully_qualified_id)
        {
            assert(!m_type.get_semantic().is_resolved());
            std::string ref_name = m_type.get_semantic().get_ref_name();
            std::string symbol = ref_name.find(".") != std::string::npos
                ? ref_name : fully_qualified_id + "." + ref_name;
            auto iter = symbols.get_symbol(symbol);
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
        type_ref m_type;

        std::shared_ptr<method_model> m_add_method;
        std::shared_ptr<method_model> m_remove_method;
    };
}
