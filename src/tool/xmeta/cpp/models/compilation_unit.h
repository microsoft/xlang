#pragma once

#include <assert.h>
#include <string_view>
#include <vector>
#include "base_model.h"
#include  "meta_reader.h"

namespace xlang::xmeta
{
    xlang::meta::reader::ElementType to_ElementType(enum_type arg);

    xlang::meta::reader::ElementType  to_ElementType(fundamental_type arg);

    fundamental_type to_fundamental_type(xlang::meta::reader::ElementType arg);

    struct symbol_table
    {
        symbol_table() = delete;
        symbol_table(std::vector<std::string> const& path) : cache{ path }
        { }

        semantic_error set_symbol(std::string_view symbol, type_category const& class_type);

        type_category get_symbol(std::string const& symbol);

    private:
        std::map<std::string, type_category> table;
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
