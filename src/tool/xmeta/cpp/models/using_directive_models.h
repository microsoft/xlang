#pragma once

#include <cassert>
#include <variant>
#include <string>
#include <string_view>

#include "base_model.h"

namespace xlang::xmeta
{
    struct namespace_model;

    struct using_alias_directive_model : base_model
    {
        using_alias_directive_model() = delete;
        using_alias_directive_model(std::string_view const& id, size_t decl_line, std::string_view const& assembly_name, std::string_view const& type_or_ns_name) :
            base_model{ id, decl_line, assembly_name },
            m_type_or_namespace_ref{ type_or_ns_name }
        { }

        bool is_namespace_ref() const noexcept
        {
            return m_type_or_namespace_ref.is_resolved() && 
                std::holds_alternative<std::shared_ptr<namespace_model>>(m_type_or_namespace_ref.get_resolved_target());
        }

        bool is_type_ref() const noexcept
        {
            return m_type_or_namespace_ref.is_resolved() &&
                std::holds_alternative<type_semantics>(m_type_or_namespace_ref.get_resolved_target());
        }

        auto const& get_type_or_namespace_ref() const noexcept
        {
            return m_type_or_namespace_ref;
        }

        void set_type_ref(type_semantics const& sem) noexcept
        {
            assert(is_type_ref());
            m_type_or_namespace_ref.resolve(sem);
        }

        void set_namespace_ref(std::shared_ptr<namespace_model> const& nm) noexcept
        {
            assert(is_namespace_ref());
            m_type_or_namespace_ref.resolve(nm);
        }

    private:
        model_ref<type_semantics, std::shared_ptr<namespace_model>> m_type_or_namespace_ref;
    };

    struct using_namespace_directive_model : base_model
    {
        using_namespace_directive_model() = delete;
        using_namespace_directive_model(std::string_view const& id, size_t decl_line, std::string_view const& assembly_name, std::string_view const& ns_name) :
            base_model{ id, decl_line, assembly_name },
            m_namespace_ref{ ns_name }
        { }

        auto const& get_namespace_ref() noexcept
        {
            return m_namespace_ref;
        }

        void set_namespace_ref(std::shared_ptr<namespace_model> const& nm) noexcept
        {
            m_namespace_ref.resolve(nm);
        }

    private:
        model_ref<std::shared_ptr<namespace_model>> m_namespace_ref;
    };
}
