#include "xmeta_models.h"

namespace xlang::xmeta
{
    void class_model::add_class_base_ref(std::string_view const& class_base_ref)
    {
        m_class_base_ref = type_ref{ class_base_ref };
    }

    void class_model::add_static_interface_ref(std::shared_ptr<interface_model> const& static_interface_ref)
    {
        m_static_interfaces.emplace_back(static_interface_ref);
    }

    void class_model::add_protected_interface_ref(std::shared_ptr<interface_model> const& protected_interface_ref)
    {
        m_protected_interfaces.emplace_back(protected_interface_ref);
    }

    void class_model::add_factory_interface_ref(std::shared_ptr<interface_model> const& static_interface_ref)
    {
        m_static_interfaces.emplace_back(static_interface_ref);
    }

    void class_model::add_overrides_interface_ref(std::shared_ptr<interface_model> const& static_interface_ref)
    {
        m_static_interfaces.emplace_back(static_interface_ref);
    }

    void class_model::add_instance_interface_ref(std::shared_ptr<interface_model> const& static_interface_ref)
    {
        m_static_interfaces.emplace_back(static_interface_ref);
    }

    std::set<std::shared_ptr<class_or_interface_model>> class_model::get_all_bases()
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
        if (m_class_base_ref == std::nullopt)
        {
            return bases;
        }
        assert(m_class_base_ref->get_semantic().is_resolved());
        auto const& class_base = std::get<std::shared_ptr<class_model>>(m_class_base_ref->get_semantic().get_resolved_target());
        std::set<std::shared_ptr<class_or_interface_model>> super_bases = class_base->get_all_bases();
        for (auto const& iter : super_bases)
        {
            bases.insert(iter);
        }
        bases.insert(class_base);
        return bases;
    }


    void class_model::validate(xlang_error_manager & error_manager)
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

    void class_model::resolve(symbol_table & symbols, xlang_error_manager & error_manager)
    {
        if (m_class_base_ref != std::nullopt)
        {
            assert(!m_class_base_ref->get_semantic().is_resolved());
            std::string ref_name = m_class_base_ref->get_semantic().get_ref_name();
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
                    m_class_base_ref->set_semantic(iter);
                }
                else
                {
                    error_manager.write_not_a_class_error(get_decl_line(), symbol);
                }
            }
        }
        class_or_interface_model::resolve(symbols, error_manager);
    }

    bool class_model::has_circular_inheritance(xlang_error_manager & error_manager)
    {
        if (contains_itself)
        {
            return true;
        }
        for (auto const& interface_base : m_interface_base_refs)
        {
            if (std::get<std::shared_ptr<interface_model>>(
                interface_base.get_semantic().get_resolved_target())->has_circular_inheritance(error_manager))
            {
                return true;
            }
        }

        std::string symbol = this->get_fully_qualified_id();
        std::set<std::string> symbol_set{ symbol };
        if (has_circular_inheritance(symbol_set, error_manager))
        {
            contains_itself = true;
            error_manager.write_struct_field_error(get_decl_line(), std::string(symbol));
        }
        return contains_itself;
    }

    bool class_model::has_circular_inheritance(std::set<std::string> & symbol_set, xlang_error_manager & error_manager)
    {
        if (contains_itself)
        {
            return true;
        }
        if (m_class_base_ref == std::nullopt)
        {
            return false;
        }
        if (m_class_base_ref->get_semantic().is_resolved())
        {
            return false;
        }
        auto const& type = m_class_base_ref->get_semantic().get_resolved_target();
        if (std::holds_alternative<std::shared_ptr<class_model>>(type))
        {
            std::shared_ptr<class_model> class_base = std::get<std::shared_ptr<class_model>>(type);
            if (!symbol_set.insert(class_base->get_fully_qualified_id()).second
                || class_base->has_circular_inheritance(symbol_set, error_manager))
            {
                return true;
            }
        }
        return false;
    }
}
