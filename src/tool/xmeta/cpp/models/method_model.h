#pragma once

#include <optional>
#include <string_view>
#include <vector>

#include "base_model.h"
#include "formal_parameter_model.h"
#include "model_types.h"

namespace xlang::xmeta
{
    struct method_semantics
    {
        bool is_protected = false;
        bool is_static = false;
    };

    struct method_model : base_model
    {
        method_model() = delete;
        method_model(std::string_view const& id, size_t decl_line, std::string_view const& assembly_name, std::optional<type_ref>&& return_type, std::vector<formal_parameter_model>&& formal_params, method_semantics const& sem) :
            base_model{ id, decl_line, assembly_name },
            m_formal_parameters{ std::move(formal_params) },
            m_return_type{ std::move(return_type) },
            m_semantic{ sem }
        { }

        method_model(std::string_view const& id, size_t decl_line, std::string_view const& assembly_name, std::optional<type_ref>&& return_type, std::vector<formal_parameter_model>&& formal_params, std::string_view const& overridden_method_ref) :
            base_model{ id, decl_line, assembly_name },
            m_formal_parameters{ std::move(formal_params) },
            m_return_type{ std::move(return_type) },
            m_implemented_method_ref{ std::string(overridden_method_ref) }
        { }

        auto const& get_formal_params() const noexcept
        {
            return m_formal_parameters;
        }

        auto const& get_overridden_method_ref() const noexcept
        {
            return m_implemented_method_ref;
        }

        auto const& get_return_type() const noexcept
        {
            return m_return_type;
        }

        auto const& get_semantic() const noexcept
        {
            return m_semantic;
        }

        void set_overridden_method_ref(std::shared_ptr<method_model> const& ref) noexcept
        {
            m_implemented_method_ref = ref;
        }

    private:
        method_semantics m_semantic;
        std::optional<type_ref> m_return_type;
        std::vector<formal_parameter_model> m_formal_parameters;
        model_ref<std::shared_ptr<method_model>> m_implemented_method_ref;
        // TODO: Add type parameters (generic types)
    };
}
