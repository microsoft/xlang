#pragma once

#include <memory>
#include <string_view>
#include <string>
#include <vector>
#include <variant>

#include "interface_model.h"
#include "method_model.h"
#include "model_ref.h"
#include "class_or_interface_model.h"
#include "property_model.h"

namespace xlang::xmeta
{
    struct class_semantics
    {
        bool is_sealed = false;
        bool is_static = false;
    };


    struct class_model : class_or_interface_model
    {
        class_model() = delete;

        class_model(std::string_view const& id, 
                size_t decl_line, 
                std::string_view const& assembly_name, 
                std::shared_ptr<namespace_body_model> const& containing_ns_body, 
                class_semantics const& sem, 
                std::string_view const& base_id) :
            class_or_interface_model{ id, decl_line, assembly_name, containing_ns_body },
            m_semantic{ sem },
            m_class_base_ref{ base_id }
        { }
        
        class_model(std::string_view const& id, 
                size_t decl_line, 
                std::string_view const& assembly_name, 
                std::shared_ptr<namespace_body_model> const& containing_ns_body, 
                class_semantics const& sem) :
            class_or_interface_model{ id, decl_line, assembly_name, containing_ns_body },
            m_semantic{ sem },
            m_class_base_ref{ "" }
        { }

        auto const& get_class_base_ref() const noexcept
        {
            return m_class_base_ref;
        }

        auto const& get_semantic() const noexcept
        {
            return m_semantic;
        }

        void add_protected_interface_ref(std::shared_ptr<interface_model> const& protected_interface_ref)
        {
            m_protected_interfaces.emplace_back(protected_interface_ref);
        }

        void add_static_interface_ref(std::shared_ptr<interface_model> const& static_interface_ref)
        {
            m_static_interfaces.emplace_back(static_interface_ref);
        }

    private:
        model_ref<std::shared_ptr<class_model>> m_class_base_ref;
        class_semantics m_semantic;
        // TODO: Add type parameters (generic types)

        std::vector<std::shared_ptr<interface_model>> m_static_interfaces;
        std::vector<std::shared_ptr<interface_model>> m_protected_interfaces;
    };
}
