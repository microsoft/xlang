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

namespace xlang::xmeta
{
    struct compilation_unit
    {
        compilation_unit() = delete;
        compilation_unit(std::string_view const& idl_assembly_name) : m_assembly{ idl_assembly_name }
        { }

        bool set_symbol(std::string_view symbol, class_type_semantics const& class_type)
        {
            return symbols.insert(std::pair<std::string, class_type_semantics>(symbol, class_type)).second;
        }

        bool set_imported_type_ref(std::string_view symbol)
        {   
            if (symbols.find(std::string(symbol)) != symbols.end())
            {
                return false;
            }
            return imported_type_refs.emplace(symbol, type_ref{ symbol }).second;
        }

        std::map<std::string_view, std::shared_ptr<namespace_model>, std::less<>> namespaces;
        std::string m_assembly;
        std::vector<std::string> m_imported_assembly_names;

        std::map<std::string, class_type_semantics> symbols;
        std::map<std::string_view, type_ref> imported_type_refs;
    };
}
