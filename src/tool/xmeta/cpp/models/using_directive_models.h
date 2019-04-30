#pragma once

#include <assert.h>
#include <variant>
#include <string>
#include <string_view>

#include "base_model.h"
#include "model_types.h"

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

        auto const& get_type_or_namespace_ref() const noexcept
        {
            return m_type_or_namespace_ref;
        }

        void set_namespace_ref(std::shared_ptr<class_model> const& nm) noexcept
        {
            assert(m_type_or_namespace_ref.holds_type<std::shared_ptr<class_model>>());
            m_type_or_namespace_ref.resolve(nm);
        }

        void set_namespace_ref(std::shared_ptr<enum_model> const& nm) noexcept
        {
            assert(m_type_or_namespace_ref.holds_type<std::shared_ptr<enum_model>>());
            m_type_or_namespace_ref.resolve(nm);
        }

        void set_namespace_ref(std::shared_ptr<interface_model> const& nm) noexcept
        {
            assert(m_type_or_namespace_ref.holds_type<std::shared_ptr<interface_model>>());
            m_type_or_namespace_ref.resolve(nm);
        }

        void set_namespace_ref(std::shared_ptr<struct_model> const& nm) noexcept
        {
            assert(m_type_or_namespace_ref.holds_type<std::shared_ptr<struct_model>>());
            m_type_or_namespace_ref.resolve(nm);
        }

        void set_namespace_ref(std::shared_ptr<namespace_model> const& nm) noexcept
        {
            assert(m_type_or_namespace_ref.holds_type<std::shared_ptr<namespace_model>>());
            m_type_or_namespace_ref.resolve(nm);
        }

    private:
        model_ref<
            std::shared_ptr<class_model>,
            std::shared_ptr<enum_model>,
            std::shared_ptr<interface_model>,
            std::shared_ptr<struct_model>,
            std::shared_ptr<namespace_model>> m_type_or_namespace_ref;
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
