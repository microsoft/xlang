#pragma once

#include <string_view>

#include "base_model.h"
#include "compilation_unit.h"

namespace xlang::xmeta
{
    struct property_semantics
    {
        bool is_protected = false;
        bool is_static = false;
    };

    struct property_model : base_model
    {
        property_model() = delete;
        property_model(std::string_view const& id, 
                size_t decl_line, 
                std::string_view const& assembly_name, 
                property_semantics const& sem, 
                type_ref&& type,
                std::shared_ptr<method_model> const& get_method, 
                std::shared_ptr<method_model> const& set_method) :
            base_model{ id, decl_line, assembly_name },
            m_semantic{ sem },
            m_type{ std::move(type) },
            m_get_method{ get_method },
            m_set_method{ set_method },
            m_implemented_property_ref{ "" }
        { }

        property_model(std::string_view const& id,
                size_t decl_line,
                std::string_view const& assembly_name,
                property_semantics const& sem,
                type_ref&& type) :
            base_model{ id, decl_line, assembly_name },
            m_type{ std::move(type) },
            m_semantic{ sem },
            m_implemented_property_ref{ "" }
        { }

        auto const& get_semantic() const noexcept
        {
            return m_semantic;
        }

        auto const& get_type() const noexcept
        {
            return m_type;
        }

        auto const& get_get_method() const noexcept
        {
            return m_get_method;
        }

        auto const& get_set_method() const noexcept
        {
            return m_set_method;
        }

        void set_overridden_property_ref(std::shared_ptr<property_model> const& ref) noexcept
        {
            assert(ref != nullptr);
            m_implemented_property_ref.resolve(ref);
            m_get_method->set_overridden_method_ref(ref->get_get_method());
            m_set_method->set_overridden_method_ref(ref->get_set_method());
        }

        compilation_error set_get_method(std::shared_ptr<method_model> const& m)
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

        compilation_error set_set_method(std::shared_ptr<method_model> const& m)
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

        void validate(xlang_error_manager & error_manager)
        {
            if (!m_get_method)
            {
                error_manager.write_property_accessor_error(get_decl_line(), get_id());
            }
        }

        void resolve(symbol_table & symbols, xlang_error_manager & error_manager, std::string const& fully_qualified_id)
        {
            if (!m_type.get_semantic().is_resolved())
            {
                /* Events should not have been resolved. If it was, it means it was not a
                class type and not a delegate type */
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
                    m_type.set_semantic(iter);
                }
            }
        }

    private:
        property_semantics m_semantic;
        model_ref<std::shared_ptr<property_model>> m_implemented_property_ref;
        type_ref m_type;
        bool m_is_array;
        std::shared_ptr<method_model> m_get_method;
        std::shared_ptr<method_model> m_set_method;
    };
}
