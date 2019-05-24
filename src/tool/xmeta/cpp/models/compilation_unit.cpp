#include "compilation_unit.h"

namespace xlang::xmeta
{
    compilation_error symbol_table::set_symbol(std::string_view symbol, class_type_semantics const& class_type)
    {
        if (!table.insert(std::pair<std::string, class_type_semantics>(symbol, class_type)).second)
        {
            return compilation_error::symbol_exists;
        }
        return compilation_error::passed;
    }

    class_type_semantics symbol_table::get_symbol(std::string const& symbol)
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

    void symbol_table::print_metadata(xlang::meta::reader::cache const& cache)
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

}