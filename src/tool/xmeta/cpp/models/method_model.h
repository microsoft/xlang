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
        method_model(std::string_view const& id, size_t decl_line, 
                std::string_view const& assembly_name, 
                std::optional<type_ref>&& return_type, 
                std::vector<formal_parameter_model>&& formal_params,
                std::variant<std::shared_ptr<class_model>, std::shared_ptr<interface_model>> const& parent,
                method_semantics const& sem) :
            base_model{ id, decl_line, assembly_name },
            m_formal_parameters{ std::move(formal_params) },
            m_return_type{ std::move(return_type) },
            m_parent{ parent },
            m_semantic{ sem }
        { }

        method_model(std::string_view const& id, 
                size_t decl_line, 
                std::string_view const& assembly_name, 
                std::optional<type_ref>&& return_type, 
                std::vector<formal_parameter_model>&& formal_params,
                std::variant<std::shared_ptr<class_model>, std::shared_ptr<interface_model>> const& parent,
                std::string_view const& overridden_method_ref) :
            base_model{ id, decl_line, assembly_name },
            m_formal_parameters{ std::move(formal_params) },
            m_return_type{ std::move(return_type) },
            m_parent{ parent },
            m_overridden_method_ref{ std::string(overridden_method_ref) }
        { }

        auto const& get_formal_parameters() const noexcept
        {
            return m_formal_parameters;
        }

        auto const& get_overridden_method_ref() const noexcept
        {
            return m_overridden_method_ref;
        }

        auto const& get_return_type() const noexcept
        {
            return m_return_type;
        }

        auto const& get_semantic() const noexcept
        {
            return m_semantic;
        }

        auto const& get_parent() const noexcept
        {
            return m_parent;
        }

        void set_overridden_method_ref(std::shared_ptr<method_model> const& ref) noexcept
        {
            m_overridden_method_ref = ref;
        }

    private:
        method_semantics m_semantic;
        std::optional<type_ref> m_return_type;
        std::vector<formal_parameter_model> m_formal_parameters;
        std::variant<std::string, std::shared_ptr<method_model>> m_overridden_method_ref;
        std::variant<std::shared_ptr<class_model>, std::shared_ptr<interface_model>> m_parent;
        // TODO: Add type parameters (generic types)
    };
}
