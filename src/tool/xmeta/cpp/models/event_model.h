#pragma once

#include <string_view>

#include "base_model.h"
#include "model_types.h"

namespace xlang::xmeta
{
    struct event_modifier
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
                event_modifier const& sem, 
                std::shared_ptr<method_model> const& add_method, 
                std::shared_ptr<method_model> const& remove_method,
                type_ref&& t) :
            base_model{ id, decl_line, assembly_name },
            m_modifier{ sem },
            m_add_method{ add_method },
            m_remove_method{ remove_method },
            m_type{ std::move(t) },
            m_implemented_event_ref{ "" }
        { }

        event_model(std::string_view const& id,
                size_t decl_line,
                std::string_view const& assembly_name,
                event_modifier const& sem,
                type_ref&& t) :
            base_model{ id, decl_line, assembly_name },
            m_modifier{ sem },
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
            return m_modifier;
        }

        auto const& get_type() const noexcept
        {
            return m_type;
        }

        void set_overridden_event_ref(std::shared_ptr<event_model> const& ref) noexcept;

        compilation_error set_add_method(std::shared_ptr<method_model> const& m);

        compilation_error set_remove_method(std::shared_ptr<method_model> const& m);

        void resolve(symbol_table & symbols, xlang_error_manager & error_manager, std::string const& fully_qualified_id);

    private:
        event_modifier m_modifier;
        model_ref<std::shared_ptr<event_model>> m_implemented_event_ref;
        type_ref m_type;
        std::shared_ptr<method_model> m_add_method = nullptr;
        std::shared_ptr<method_model> m_remove_method = nullptr;
    };
}
