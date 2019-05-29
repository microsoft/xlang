#pragma once

#include <assert.h>
#include <string_view>
#include <vector>
#include "base_model.h"

namespace xlang::xmeta
{
    struct symbol_table
    {
        symbol_table() = delete;
        symbol_table(std::vector<std::string> const& path) : cache{ path }
        { }

        compilation_error set_symbol(std::string_view symbol, class_type_semantics const& class_type);

        class_type_semantics get_symbol(std::string const& symbol);

    private:
        std::map<std::string, class_type_semantics> table;
        xlang::meta::reader::cache cache;

        void print_metadata(xlang::meta::reader::cache const& cache);
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
