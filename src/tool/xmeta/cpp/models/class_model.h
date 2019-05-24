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

        void add_class_base_ref(std::string_view const& class_base_ref)
        {
            m_class_base_ref = type_ref{ class_base_ref };
        }

        void add_static_interface_ref(std::shared_ptr<interface_model> const& static_interface_ref)
        {
            m_static_interfaces.emplace_back(static_interface_ref);
        }

        void add_protected_interface_ref(std::shared_ptr<interface_model> const& protected_interface_ref)
        {
            m_protected_interfaces.emplace_back(protected_interface_ref);
        }

        void add_factory_interface_ref(std::shared_ptr<interface_model> const& static_interface_ref)
        {
            m_static_interfaces.emplace_back(static_interface_ref);
        }

        void add_overrides_interface_ref(std::shared_ptr<interface_model> const& static_interface_ref)
        {
            m_static_interfaces.emplace_back(static_interface_ref);
        }

        void add_instance_interface_ref(std::shared_ptr<interface_model> const& static_interface_ref)
        {
            m_static_interfaces.emplace_back(static_interface_ref);
        }

        std::set<std::shared_ptr<class_or_interface_model>> get_all_bases()
        {
            std::set<std::shared_ptr<class_or_interface_model>> bases;
            for (auto const& base : this->get_interface_bases())
            {
                auto const& type = base.get_semantic().get_resolved_target();
                assert(base.get_semantic().is_resolved());
                assert(std::holds_alternative<std::shared_ptr<interface_model>>(type));
                std::shared_ptr<interface_model> const& interface_base = std::get<std::shared_ptr<interface_model>>(base.get_semantic().get_resolved_target());
                bases.insert(interface_base);
                std::set<std::shared_ptr<interface_model>> super_bases = interface_base->get_all_interface_bases();
                for (auto const& iter : super_bases)
                {
                    bases.insert(iter);
                }
            }
            assert(m_class_base_ref.get_semantic().is_resolved());
            auto const& class_base = std::get<std::shared_ptr<class_model>>(m_class_base_ref.get_semantic().get_resolved_target());
            std::set<std::shared_ptr<class_or_interface_model>> super_bases = class_base->get_all_bases();
            for (auto const& iter : super_bases)
            {
                bases.insert(iter);
            }
            bases.insert(class_base);
            return bases;
        }


        void validate(xlang_error_manager & error_manager)
        {
            std::set<std::shared_ptr<class_or_interface_model>> bases = get_all_bases();
            for (auto const& base : bases)
            {
                for (auto const& base_event : base->get_events())
                {
                    if (member_id_exists(base_event->get_id()))
                    {
                        error_manager.write_type_member_exists_error(get_decl_line(), base_event->get_id(), get_fully_qualified_id());
                        return;
                    }
                }

                for (auto const& base_properties : base->get_properties())
                {
                    if (member_id_exists(base_properties->get_id()))
                    {
                        error_manager.write_type_member_exists_error(get_decl_line(), base_properties->get_id(), get_fully_qualified_id());
                        return;
                    }
                }

                for (auto const& base_method : base->get_methods())
                {
                    if (member_id_exists(base_method->get_id()))
                    {
                        error_manager.write_type_member_exists_error(get_decl_line(), base_method->get_id(), get_fully_qualified_id());
                        return;
                    }
                }
            }
            class_or_interface_model::validate(error_manager);
        }

        void resolve(symbol_table & symbols, xlang_error_manager & error_manager)
        {
            assert(!m_class_base_ref.get_semantic().is_resolved());
            std::string ref_name = m_class_base_ref.get_semantic().get_ref_name();
            std::string symbol = ref_name.find(".") != std::string::npos
                ? ref_name : this->get_containing_namespace_body()->get_containing_namespace()->get_fully_qualified_id() + "." + ref_name;

            auto iter = symbols.get_symbol(symbol);
            if (std::holds_alternative<std::monostate>(iter))
            {
                error_manager.write_unresolved_type_error(get_decl_line(), symbol);
            }
            else
            {
                if (std::holds_alternative<std::shared_ptr<class_model>>(iter))
                {
                    m_class_base_ref.set_semantic(iter);
                }
                else
                {
                    error_manager.write_not_a_class_error(get_decl_line(), symbol);
                }
            }
            class_or_interface_model::resolve(symbols, error_manager);
        }

    private:
        type_ref m_class_base_ref;
        class_semantics m_semantic;
        // TODO: Add type parameters (generic types)

        std::vector<std::shared_ptr<interface_model>> m_static_interfaces;
        std::vector<std::shared_ptr<interface_model>> m_protected_interfaces;
        std::vector<std::shared_ptr<interface_model>> m_factory_interfaces;
        std::vector<std::shared_ptr<interface_model>> m_overrides_interfaces;
        std::vector<std::shared_ptr<interface_model>> m_instance_interfaces;
    };
}
