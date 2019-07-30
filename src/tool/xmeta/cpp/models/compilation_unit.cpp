#include "compilation_unit.h"

namespace xlang::xmeta
{
    using namespace xlang::meta::reader;

    ElementType to_ElementType(enum_type arg)
    {
        switch (arg)
        {
        case enum_type::Int8:
            return ElementType::I1;
        case enum_type::UInt8:
            return ElementType::U1;
        case enum_type::Int16:
            return ElementType::I2;
        case enum_type::UInt16:
            return ElementType::U2;
        case enum_type::Int32:
            return ElementType::I4;
        case enum_type::UInt32:
            return ElementType::U4;
        case enum_type::Int64:
            return ElementType::I8;
        case enum_type::UInt64:
            return ElementType::U8;
        default:
            XLANG_ASSERT(false);
            return ElementType::Void;
        }
    }

    semantic_error symbol_table::set_symbol(std::string_view symbol, type_category const& class_type)
    {
        if (!table.insert(std::pair<std::string, type_category>(symbol, class_type)).second)
        {
            return semantic_error::symbol_exists;
        }
        return semantic_error::passed;
    }

    type_category symbol_table::get_symbol(std::string const& symbol)
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

    void symbol_table::print_metadata(xlang::meta::reader::cache const& c)
    {
        for (auto const& db : c.databases())
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