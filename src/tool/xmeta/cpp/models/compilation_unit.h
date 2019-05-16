#pragma once

#include <assert.h>
#include <string_view>
#include <vector>
#include "meta_reader.h"
#include "base_model.h"

namespace xlang::xmeta
{
    enum class compilation_error : bool
    {
        passed = true,
        symbol_exists = false
    };

    struct symbol_table
    {
        symbol_table() = delete;
        symbol_table(std::vector<std::string> const& path) : cache{ path }
        { }

        compilation_error set_symbol(std::string_view symbol, class_type_semantics const& class_type)
        {
            if (!table.insert(std::pair<std::string, class_type_semantics>(symbol, class_type)).second)
            {
                return compilation_error::symbol_exists;
            }
            return compilation_error::passed;
        }

        class_type_semantics get_symbol(std::string const& symbol)
        {
            auto iter = table.find(symbol);
            if (iter == table.end())
            {
                auto const& type_def = cache.find(symbol);
                if (type_def)
                {
                    std::shared_ptr<xlang::meta::reader::TypeDef> type = std::make_shared<xlang::meta::reader::TypeDef>(std::move(type_def));
                    table[symbol] = type;
                    return type;
                }
                return std::monostate();
            }
            return iter->second;
        }

    private:
        std::map<std::string, class_type_semantics> table;
        xlang::meta::reader::cache cache;

        void print_metadata(xlang::meta::reader::cache const& cache)
        {
            for (auto const& db : cache.databases())
            {
                std::cout << "TypeDefs ------------------ " << std::endl;
                for (auto const& type_def : db.TypeDef)
                {
                    std::cout << "assembly: " << type_def.get_database().Assembly[0].Name() << std::endl;

                    std::cout << type_def.TypeNamespace() << "." << type_def.TypeName() << std::endl;
                    if (type_def.Extends())
                    {
                        std::cout << "  extends: " << type_def.Extends().TypeRef().TypeName() << std::endl;
                        std::cout << "           " << type_def.is_struct() << std::endl;
                    }
                    if (size(type_def.MethodList()) > 0)
                    {
                        for (auto const& method : type_def.MethodList())
                        {
                            std::cout << "       " << method.Name() << std::endl;
                        }
                    }
                }
                std::cout << "TypeRefs ------------------ " << std::endl;
                for (auto const& type_ref : db.TypeRef)
                {
                    std::cout << type_ref.TypeNamespace() << "." << type_ref.TypeName() << std::endl;
                }
            }
        }
    };


    struct compilation_unit
    {
        compilation_unit() = delete;
        compilation_unit(std::string_view const& idl_assembly_name, std::vector<std::string> const& path) : m_assembly{ idl_assembly_name }, symbols{ path }
        { }

        std::map<std::string_view, std::shared_ptr<namespace_model>, std::less<>> namespaces;
        std::string m_assembly;
        std::vector<std::string> m_imported_assembly_names_paths;
        symbol_table symbols;
    };
}
