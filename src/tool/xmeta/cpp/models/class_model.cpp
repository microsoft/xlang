#include "xmeta_models.h"

namespace xlang::xmeta
{
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
                if (member_exists(base_event->get_name()))
                {
                    error_manager.report_error(idl_error::DUPLICATE_TYPE_MEMBER_ID, get_decl_line(), base_event->get_name());
                    return;
                }
            }

            for (auto const& base_properties : base->get_properties())
            {
                if (member_exists(base_properties->get_name()))
                {
                    error_manager.report_error(idl_error::DUPLICATE_TYPE_MEMBER_ID, get_decl_line(), base_properties->get_name());
                    return;
                }
            }

            for (auto const& base_method : base->get_methods())
            {
                if (member_exists(base_method->get_name()))
                {
                    error_manager.report_error(idl_error::DUPLICATE_TYPE_MEMBER_ID, get_decl_line(), base_method->get_name());
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
                ? ref_name : this->get_containing_namespace_body()->get_containing_namespace()->get_qualified_name() + "." + ref_name;

            auto iter = symbols.get_symbol(symbol);
            if (std::holds_alternative<std::monostate>(iter))
            {
                error_manager.report_error(idl_error::UNRESOLVED_TYPE, get_decl_line(), symbol);
            }
            else
            {
                if (std::holds_alternative<std::shared_ptr<class_model>>(iter))
                {
                    m_class_base_ref->set_semantic(iter);
                }
                else if (std::holds_alternative<std::shared_ptr<xlang::meta::reader::TypeDef>>(iter))
                {
                    auto const& type_def = std::get<std::shared_ptr<xlang::meta::reader::TypeDef>>(iter);
                    if (type_def->is_runtime_class())
                    {
                        m_class_base_ref->set_semantic(iter);
                    }
                    else
                    {
                        error_manager.report_error(idl_error::TYPE_NOT_CLASS, get_decl_line(), symbol);
                    }
                }
                else
                {
                    error_manager.report_error(idl_error::TYPE_NOT_CLASS, get_decl_line(), symbol);
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

        std::string symbol = this->get_qualified_name();
        std::set<std::string> symbol_set{ symbol };
        if (has_circular_inheritance(symbol_set, error_manager))
        {
            contains_itself = true;
            error_manager.report_error(idl_error::CIRCULAR_STRUCT_FIELD, get_decl_line(), symbol);
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
            if (!symbol_set.insert(class_base->get_qualified_name()).second
                || class_base->has_circular_inheritance(symbol_set, error_manager))
            {
                return true;
            }
        }
        return false;
    }
}
