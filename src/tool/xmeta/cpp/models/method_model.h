#pragma once

#include <optional>
#include <string_view>
#include <vector>

#include "base_model.h"
#include "compilation_unit.h"
#include "formal_parameter_model.h"

namespace xlang::xmeta
{
    struct method_semantics
    {
        bool is_protected = false;
        bool is_static = false;
        bool is_overridable = false;
    };

    struct method_model : base_model
    {
        method_model() = delete;
        method_model(std::string_view const& id, 
                size_t decl_line, 
                std::string_view const& assembly_name, 
                std::optional<type_ref>&& return_type, 
                std::vector<formal_parameter_model>&& formal_params,
                method_semantics const& sem,
                method_association const& assoc) :
            base_model{ id, decl_line, assembly_name },
            m_formal_parameters{ std::move(formal_params) },
            m_return_type{ std::move(return_type) },
            m_semantic{ sem },
            m_association{ assoc },
            m_implemented_method_ref{ "" }
        { }

        method_model(std::string_view const& id,
                size_t decl_line,
                std::string_view const& assembly_name,
                std::optional<type_ref>&& return_type,
                method_association const& assoc) :
            base_model{ id, decl_line, assembly_name },
            m_return_type{ std::move(return_type) },
            m_association{ assoc },
            m_implemented_method_ref{ "" }
        { }

        method_model(std::string_view const& id,
                size_t decl_line,
                std::string_view const& assembly_name,
                std::optional<type_ref>&& return_type,
                method_semantics const& sem,
                method_association const& assoc) :
            base_model{ id, decl_line, assembly_name },
            m_return_type{ std::move(return_type) },
            m_semantic{ sem },
            m_association{ assoc },
            m_implemented_method_ref{ "" }
        { }

        method_model(std::string_view const& id, 
                size_t decl_line, 
                std::string_view const& assembly_name, 
                std::optional<type_ref>&& return_type, 
                std::vector<formal_parameter_model>&& formal_params,
                std::string_view const& overridden_method_ref,
                method_association const& assoc) :
            base_model{ id, decl_line, assembly_name },
            m_formal_parameters{ std::move(formal_params) },
            m_return_type{ std::move(return_type) },
            m_association{ assoc },
            m_implemented_method_ref{ std::string(overridden_method_ref) }
        { }

        method_model(std::string_view const& id,
                method_semantics const& sem,
                std::optional<type_ref>&& return_type,
                std::vector<formal_parameter_model>&& formal_params) : 
            base_model{ id, 0, "test only" },
            m_semantic{ sem },
            m_formal_parameters{ std::move(formal_params) },
            m_return_type{ std::move(return_type) },
            m_implemented_method_ref{ "" }
        { }


        auto const& get_method_association()
        {
            return m_association;
        }

        auto const& get_formal_parameters() const noexcept
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

        void add_formal_parameter(formal_parameter_model&& formal_param)
        {
            m_formal_parameters.emplace_back(std::move(formal_param));
        }

        void set_overridden_method_ref(std::shared_ptr<method_model> const& ref) noexcept;

        void resolve(symbol_table & symbols, xlang_error_manager & error_manager, std::string const& fully_qualified_id);

    private:
        method_semantics m_semantic;
        std::optional<type_ref> m_return_type;
        std::vector<formal_parameter_model> m_formal_parameters;
        model_ref<std::shared_ptr<method_model>> m_implemented_method_ref;
        method_association m_association;
        // TODO: Add type parameters (generic types)
    };
}
