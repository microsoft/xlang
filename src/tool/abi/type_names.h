#pragma once

#include <string>

#include "common.h"
#include "meta_reader.h"
#include "namespace_iterator.h"

inline std::string clr_full_name(xlang::meta::reader::TypeDef const& type)
{
    std::string result;
    result.reserve(type.TypeNamespace().length() + type.TypeName().length() + 1);
    result += type.TypeNamespace();
    result += '.';
    result += type.TypeName();
    return result;
}

namespace details
{
    inline void write_type_prefix(std::string& result, xlang::meta::reader::category typeCategory)
    {
        if (typeCategory == xlang::meta::reader::category::delegate_type)
        {
            result += 'I';
        }
    }

    inline void write_type_prefix(std::string& result, xlang::meta::reader::TypeDef const& type)
    {
        write_type_prefix(result, xlang::meta::reader::get_category(type));
    }
}

inline std::string cpp_type_name(xlang::meta::reader::TypeDef const& type)
{
    std::string result;
    details::write_type_prefix(result, type);
    result += type.TypeName();
    return result;
}

namespace details
{
    template <bool IsGenericParam>
    void write_mangled_name(std::string& result, std::string_view name)
    {
        result.reserve(result.length() + name.length());
        for (auto ch : name)
        {
            if (ch == '.')
            {
                result += IsGenericParam ? "__C" : "_C";
            }
            else if (ch == '_')
            {
                result += IsGenericParam ? "__z" : "__";
            }
            else if (ch == '`')
            {
                result += '_';
            }
            else
            {
                result += ch;
            }
        }
    }
}

template <bool IsGenericParam>
inline std::string mangled_name(xlang::meta::reader::TypeDef const& type)
{
    std::string result;
    if (!is_generic(type))
    {
        details::write_mangled_name<IsGenericParam>(result, type.TypeNamespace());
        result += IsGenericParam ? "__C" : "_C";
    }
    else
    {
        // Generic types don't have the namespace included in the mangled name
        result += "__F";
    }

    details::write_type_prefix(result, type);
    details::write_mangled_name<IsGenericParam>(result, type.TypeName());
    return result;
}

inline std::string_view element_type_signature(xlang::meta::reader::ElementType type)
{
    using namespace std::literals;
    using namespace xlang::meta::reader;

    switch (type)
    {
    case ElementType::Boolean: return "b1"sv;
    case ElementType::Char: return "c2"sv;
    case ElementType::U1: return "u1"sv;
    case ElementType::I2: return "i2"sv;
    case ElementType::U2: return "u2"sv;
    case ElementType::I4: return "i4"sv;
    case ElementType::U4: return "u4"sv;
    case ElementType::I8: return "i8"sv;
    case ElementType::U8: return "u8"sv;
    case ElementType::R4: return "f4"sv;
    case ElementType::R8: return "f8"sv;
    case ElementType::String: return "string"sv;
    case ElementType::Object: return "cinterface(IInspectable)"sv;
    }

    xlang::throw_invalid("Unrecognized ElementType: ", std::to_string(static_cast<int>(type)));
}

inline void append_enum_signature(std::string& result, xlang::meta::reader::TypeDef const& type)
{
    XLANG_ASSERT(get_category(type) == xlang::meta::reader::category::enum_type);
    auto ns = type.TypeNamespace();
    auto name = type.TypeName();

    result.reserve(result.length() + 10 + ns.length() + name.length());
    result += "enum(";
    result += ns;
    result.push_back('.');
    result += name;
    result.push_back(';');
    result += element_type_signature(underlying_enum_type(type));
    result.push_back(')');
}

inline std::string enum_signature(xlang::meta::reader::TypeDef const& type)
{
    std::string result;
    append_enum_signature(result, type);
    return result;
}

inline void append_struct_signature(std::string& result, xlang::meta::reader::TypeDef const& type)
{
    using namespace std::literals;
    using namespace xlang::meta::reader;

    result += "struct("sv;
    result += type.TypeNamespace();
    result.push_back('.');
    result += type.TypeName();

    for (auto const& field : type.FieldList())
    {
        auto handle_generic_inst = [&](GenericTypeInstSig const&)
        {
            XLANG_ASSERT(false);
            xlang::throw_invalid("Invalid member '", field.Name(), "' of struct '", type.TypeNamespace(), ".",
                type.TypeName(), "'. Structs cannot have generic instantiations as members");
        };

        auto handle_type_def = [&](TypeDef const& def)
        {
            switch (get_category(def))
            {
            case category::enum_type:
                append_enum_signature(result, def);
                break;

            case category::struct_type:
                append_struct_signature(result, def);
                break;

            default:
                xlang::throw_invalid("Invalid member '", field.Name(), "' of struct '", type.TypeNamespace(), ".",
                    type.TypeName(), "'. Struct members must be of element, enum, or struct type");
            }
        };

        auto handle_type_ref = [&](TypeRef const& ref)
        {
            if (ref.TypeNamespace() == system_namespace)
            {
                if (ref.TypeName() == "Guid"sv)
                {
                    result += "g16"sv;
                    return;
                }

                xlang::throw_invalid("Invalid type '", ref.TypeName(), "' in 'System' namespace found as member '",
                    field.Name(), "' of the struct '", type.TypeNamespace(), ".", type.TypeName(), "'");
            }

            handle_type_def(find_required(ref));
        };

        result.push_back(';');

        std::visit(xlang::visit_overload{
            [&](ElementType t)
            {
                result += element_type_signature(t);
            },
            [&](coded_index<TypeDefOrRef> const& t)
            {
                visit(t, xlang::visit_overload{
                    handle_generic_inst,
                    handle_type_ref,
                    handle_type_def});
            },
            [&](GenericTypeIndex)
            {
                XLANG_ASSERT(false);
                xlang::throw_invalid("Invalid member '", field.Name(), "' of struct '", type.TypeNamespace(), ".",
                    type.TypeName(), "'. Structs cannot be generic types");
            },
            handle_generic_inst}, field.Signature().Type().Type());
    }

    result.push_back(')');
}

inline std::string struct_signature(xlang::meta::reader::TypeDef const& type)
{
    std::string result;
    append_struct_signature(result, type);
    return result;
}

inline void append_type_guid(std::string& result, xlang::meta::reader::TypeDef const& type)
{
    // TODO
}

inline std::string delegate_signature(xlang::meta::reader::TypeDef const& type)
{
    std::string result;
    result.reserve(result.length() + 12 + 36);
    result += "delegate({";
    append_type_guid(result, type);
    result += "})";
    return result;
}

inline std::string interface_signature(xlang::meta::reader::TypeDef const& type)
{
    std::string result;
    result.reserve(result.length() + 2 + 36);
    result.push_back('{');
    append_type_guid(result, type);
    result.push_back('}');
    return result;
}
