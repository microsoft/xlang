#pragma once

#include <string_view>

#include "base_model.h"
#include "compilation_unit.h"
#include "method_model.h"
#include "model_types.h"

namespace xlang::xmeta
{
    struct property_modifier
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
                property_modifier const& sem,
                type_ref&& type,
                std::shared_ptr<method_model> const& get_method,
                std::shared_ptr<method_model> const& set_method) :
            base_model{ id, decl_line, assembly_name },
            m_modifier{ sem },
            m_type{ std::move(type) },
            m_get_method{ get_method },
            m_set_method{ set_method },
            m_implemented_property_ref{ "" }
        { }

        property_model(std::string_view const& id,
                size_t decl_line,
                std::string_view const& assembly_name,
                property_modifier const& sem,
                type_ref&& type) :
            base_model{ id, decl_line, assembly_name },
            m_type{ std::move(type) },
            m_modifier{ sem },
            m_implemented_property_ref{ "" }
        { }

        auto const& get_semantic() const noexcept
        {
            return m_modifier;
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

        void set_overridden_property_ref(std::shared_ptr<property_model> const& ref) noexcept;

        compilation_error set_get_method(std::shared_ptr<method_model> const& m);

        compilation_error set_set_method(std::shared_ptr<method_model> const& m);

        void validate(xlang_error_manager & error_manager);

        void resolve(symbol_table & symbols, xlang_error_manager & error_manager, std::string const& fully_qualified_id);

    private:
        property_modifier m_modifier;
        model_ref<std::shared_ptr<property_model>> m_implemented_property_ref;
        type_ref m_type;
        bool m_is_array;
        std::shared_ptr<method_model> m_get_method;
        std::shared_ptr<method_model> m_set_method;
    };
}
