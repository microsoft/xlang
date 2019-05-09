#pragma once

#include <assert.h>
#include <string_view>
#include <vector>

#include "event_model.h"
#include "interface_model.h"
#include "method_model.h"
#include "model_ref.h"
#include "class_or_interface_model.h"
#include "property_model.h"
#include <unordered_set>

namespace xlang::xmeta
{
    struct interface_model : class_or_interface_model
    {
        interface_model() = delete;
        interface_model(std::string_view const& id, size_t decl_line, std::string_view const& assembly_name, std::shared_ptr<namespace_body_model> const& containing_ns_body) :
            class_or_interface_model{ id, decl_line, assembly_name, containing_ns_body }
        { }

        std::unordered_set<std::shared_ptr<interface_model>> get_all_interface_bases()
        {
            std::unordered_set<std::shared_ptr<interface_model>> bases;
            for (auto const& base : this->get_interface_bases())
            {
                auto const& type = base.get_semantic().get_resolved_target();
                assert(base.get_semantic().is_resolved());
                assert(std::holds_alternative<std::shared_ptr<interface_model>>(type));
                std::shared_ptr<interface_model> const& interface_base = std::get<std::shared_ptr<interface_model>>(base.get_semantic().get_resolved_target());
                bases.insert(interface_base);
                std::unordered_set<std::shared_ptr<interface_model>> super_bases = interface_base->get_all_interface_bases();
                for (auto const& iter : super_bases)
                {
                    bases.insert(iter);
                }
            }
            return bases;
        }

        void resolve(std::map<std::string, class_type_semantics> symbols, xlang_error_manager & error_manager)
        {
            class_or_interface_model::resolve(symbols, error_manager);
        }

        bool has_circular_inheritance(std::map<std::string, class_type_semantics> const& symbols, xlang_error_manager & error_manager)
        {
            if (contains_itself)
            {
                return true;
            }
            std::string symbol = this->get_fully_qualified_id();
            std::set<std::string> symbol_set{ symbol };
            if (has_circular_inheritance(symbols, symbol_set, error_manager))
            {
                contains_itself = true;
                error_manager.write_struct_field_error(get_decl_line(), std::string(symbol));
            }
            return contains_itself;
        }

        bool has_circular_inheritance(std::map<std::string, class_type_semantics> const& symbols, std::set<std::string> & symbol_set, xlang_error_manager & error_manager)
        {
            if (contains_itself)
            {
                return true;
            }
            for (auto const& base : this->get_interface_bases())
            {
                assert(base.get_semantic().is_resolved());
                auto const& type = base.get_semantic().get_resolved_target();
                if (std::holds_alternative<std::shared_ptr<interface_model>>(type))
                {
                    std::shared_ptr<interface_model> interface_base = std::get<std::shared_ptr<interface_model>>(type);
                    if (!symbol_set.insert(interface_base->get_fully_qualified_id()).second
                        || interface_base->has_circular_inheritance(symbols, symbol_set, error_manager))
                    {
                        return true;
                    }
                    symbol_set.erase(interface_base->get_fully_qualified_id());
                }
            }
            return false;
        }

    private:
        bool contains_itself = false;
    };
}
