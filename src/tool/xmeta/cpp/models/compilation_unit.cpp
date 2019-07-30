#include "compilation_unit.h"

namespace xlang::xmeta
{
    using namespace xlang::meta::reader;

    fundamental_type to_fundamental_type(xlang::meta::reader::ElementType arg)
    {
        switch (arg)
        {
        case ElementType::String:
            return fundamental_type::String;
        case ElementType::I1:
            return fundamental_type::Int8;
        case ElementType::U1:
            return fundamental_type::UInt8;
        case ElementType::I2:
            return fundamental_type::Int16;
        case ElementType::U2:
            return fundamental_type::UInt16;
        case ElementType::I4:
            return fundamental_type::Int32;
        case ElementType::U4:
            return fundamental_type::UInt32;
        case ElementType::I8:
            return fundamental_type::Int64;
        case ElementType::U8:
            return fundamental_type::UInt64;
        case ElementType::Char:
            return fundamental_type::Char16;
        case ElementType::R4:
            return fundamental_type::Single;
        case ElementType::R8:
            return fundamental_type::Double;
        case ElementType::Boolean:
            return fundamental_type::Boolean;
        default:
            XLANG_ASSERT(false);
        }
    }

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

    ElementType to_ElementType(fundamental_type arg)
    {
        switch (arg)
        {
        case fundamental_type::String:
            return ElementType::String;
        case fundamental_type::Int8:
            return ElementType::I1;
        case fundamental_type::UInt8:
            return ElementType::U1;
        case fundamental_type::Int16:
            return ElementType::I2;
        case fundamental_type::UInt16:
            return ElementType::U2;
        case fundamental_type::Int32:
            return ElementType::I4;
        case fundamental_type::UInt32:
            return ElementType::U4;
        case fundamental_type::Int64:
            return ElementType::I8;
        case fundamental_type::UInt64:
            return ElementType::U8;
        case fundamental_type::Char16:
            return ElementType::Char;
        case fundamental_type::Single:
            return ElementType::R4;
        case fundamental_type::Double:
            return ElementType::R8;
        case fundamental_type::Boolean:
            return ElementType::Boolean;
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


    semantic_error attributes_table::define_attribute_type(std::string_view name, std::shared_ptr<attribute_type_model> const& attribute_type)
    {
        if (!table.insert(std::pair<std::string, std::shared_ptr<attribute_type_model>>(name, attribute_type)).second)
        {
            return semantic_error::symbol_exists;
        }
        return semantic_error::passed;
    }

    std::shared_ptr<attribute_type_model> attributes_table::get_attribute_type(std::string const& name)
    {
        auto iter = table.find(name);
        if (iter == table.end())
        {
            return nullptr;
        }
        return iter->second;
    }

}