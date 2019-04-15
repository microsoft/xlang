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
        using_alias_directive_model(std::string_view const& id, size_t decl_line, std::string_view const& type_or_ns_name) :
            base_model{ id, decl_line },
            m_type_or_namespace_ref{ std::string(type_or_ns_name) }
        { }

        bool is_resolved() const noexcept
        {
            return !std::holds_alternative<std::string>(m_type_or_namespace_ref);
        }

        bool is_namespace_ref() const noexcept
        {
            return std::holds_alternative<std::shared_ptr<namespace_model>>(m_type_or_namespace_ref);
        }

        bool is_type_semantic() const noexcept
        {
            return std::holds_alternative<type_semantics>(m_type_or_namespace_ref);
        }

        auto const& to_string() noexcept
        {
            assert(!is_resolved());
            return std::get<std::string>(m_type_or_namespace_ref);
        }

        auto const& to_type_semantic() noexcept
        {
            assert(is_type_semantic());
            return std::get<type_semantics>(m_type_or_namespace_ref);
        }

        auto const& to_namespace_model_ref() noexcept
        {
            assert(is_namespace_ref());
            return std::get<std::shared_ptr<namespace_model>>(m_type_or_namespace_ref);
        }

        void set_type_ref(type_semantics const& sem) noexcept
        {
            m_type_or_namespace_ref = sem;
        }

        void set_namespace_ref(std::shared_ptr<namespace_model> const& nm) noexcept
        {
            m_type_or_namespace_ref = nm;
        }

    private:
        std::variant<std::string, type_semantics, std::shared_ptr<namespace_model>> m_type_or_namespace_ref;
    };

    struct using_namespace_directive_model : base_model
    {
        using_namespace_directive_model(std::string_view const& id, size_t decl_line, std::string_view const& ns_name) :
            base_model{ id, decl_line },
            m_namespace_ref{ std::string(ns_name) }
        { }
        using_namespace_directive_model(using_namespace_directive_model&& und) = default;

        bool is_resolved() const noexcept
        {
            return !std::holds_alternative<std::string>(m_namespace_ref);
        }

        auto const& to_string() noexcept
        {
            assert(!is_resolved());
            return std::get<std::string>(m_namespace_ref);
        }

        auto const& to_namespace_model_ref() noexcept
        {
            assert(is_resolved());
            return std::get<std::shared_ptr<namespace_model>>(m_namespace_ref);
        }

        void set_namespace_ref(std::shared_ptr<namespace_model> const& nm) noexcept
        {
            m_namespace_ref = nm;
        }

    private:
        std::variant<std::string, std::shared_ptr<namespace_model>> m_namespace_ref;
    };
}
