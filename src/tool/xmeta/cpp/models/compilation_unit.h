#pragma once

#include <assert.h>
#include <string_view>
#include <vector>

#include "base_model.h"

namespace xlang::xmeta
{
    struct symbol_table
    {
        std::map<std::string, class_type_semantics> table;

        bool set_symbol(std::string_view symbol, class_type_semantics const& class_type)
        {
            return table.insert(std::pair<std::string, class_type_semantics>(symbol, class_type)).second;
        }

        class_type_semantics get_symbol(std::string symbol)
        {
            auto iter = table.find(symbol);
            if (iter == table.end())
            {
                return std::monostate();
            }
            return iter->second;
        }
    };


    struct compilation_unit
    {
        compilation_unit() = delete;
        compilation_unit(std::string_view const& idl_assembly_name) : m_assembly{ idl_assembly_name }
        { }


        bool set_imported_type_ref(std::string_view symbol)
        {   
            return imported_type_refs.emplace(symbol, type_ref{ symbol }).second;
        }

        std::map<std::string_view, std::shared_ptr<namespace_model>, std::less<>> namespaces;
        std::string m_assembly;
        std::vector<std::string> m_imported_assembly_names;

        symbol_table symbols;
        std::map<std::string_view, type_ref> imported_type_refs;
    };
}
