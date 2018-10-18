#pragma once

#include "abi_writer.h"
#include "generic_arg_stack.h"
#include "meta_reader.h"
#include "sha1.h"
#include "text_writer.h"

inline constexpr std::string_view type_prefix(xlang::meta::reader::category typeCategory) noexcept
{
    using namespace xlang::meta::reader;
    switch (typeCategory)
    {
    case category::delegate_type:
        // Delegates are actually interfaces, but aren't represented that way in the metadata, so we need to prefix each
        // type name with an 'I'
        return "I";

    default:
        return "";
    }
}

inline void write_namespace_open(writer& w, std::string_view ns)
{
    if (w.config().ns_prefix_state == ns_prefix::always)
    {
        w.write("ABI::");
    }
    else if (w.config().ns_prefix_state == ns_prefix::optional)
    {
        w.write("ABI_PARAMETER(");
    }

    w.write("@::", ns);
}

inline void write_namespace_close(writer& w)
{
    if (w.config().ns_prefix_state == ns_prefix::optional)
    {
        w.write(')');
    }
}

enum class format_flags
{
    none = 0x00,
    generic_param = 0x01,       // Generic params often get handled differently (e.g. pointers, mangled names, etc.)
    ignore_namespace = 0x02,    // Only write type the type name
    function_param = 0x04,      // Function params get handled similarly, but not identically, to generic params

    generic_or_function_param = generic_param | function_param,
};
inline constexpr format_flags operator&(format_flags lhs, format_flags rhs) { return static_cast<format_flags>(static_cast<int>(lhs) & static_cast<int>(rhs)); }
inline constexpr format_flags operator|(format_flags lhs, format_flags rhs) { return static_cast<format_flags>(static_cast<int>(lhs) | static_cast<int>(rhs)); }
inline constexpr format_flags operator~(format_flags value) { return static_cast<format_flags>(~static_cast<int>(value)); }

constexpr bool flags_set(format_flags flags, format_flags mask)
{
    return (flags & mask) == mask;
}

constexpr bool any_flag_set(format_flags flags, format_flags mask)
{
    return (flags & mask) != format_flags::none;
}

constexpr bool flags_clear(format_flags flags, format_flags mask)
{
    return (flags & mask) == format_flags::none;
}

constexpr format_flags clear_flags(format_flags flags, format_flags mask)
{
    return flags & ~mask;
}

// Forward declarations due to recursive dependencies
inline void write_type_clr(writer& w, xlang::meta::reader::TypeDef const& type, generic_arg_stack const& genericArgs, format_flags flags = format_flags::none);
inline void write_type_clr(writer& w, xlang::meta::reader::TypeRef const& ref, generic_arg_stack const& genericArgs, format_flags flags = format_flags::none);
inline void write_type_clr(writer& w, xlang::meta::reader::ElementType type, generic_arg_stack const& genericArgs, format_flags flags = format_flags::none);
inline void write_type_clr(writer& w, xlang::meta::reader::coded_index<xlang::meta::reader::TypeDefOrRef> const& type, generic_arg_stack const& genericArgs, format_flags flags = format_flags::none);
inline void write_type_clr(writer& w, xlang::meta::reader::TypeSig const& type, generic_arg_stack const& genericArgs, format_flags flags = format_flags::none);
inline void write_type_clr(writer& w, xlang::meta::reader::GenericTypeInstSig const& type, generic_arg_stack const& genericArgs, format_flags flags = format_flags::none);

inline void write_type_cpp(writer& w, xlang::meta::reader::TypeDef const& type, generic_arg_stack const& genericArgs, format_flags flags = format_flags::none);
inline void write_type_cpp(writer& w, xlang::meta::reader::TypeRef const& ref, generic_arg_stack const& genericArgs, format_flags flags = format_flags::none);
inline void write_type_cpp(writer& w, xlang::meta::reader::ElementType type, generic_arg_stack const& genericArgs, format_flags flags = format_flags::none);
inline void write_type_cpp(writer& w, xlang::meta::reader::coded_index<xlang::meta::reader::TypeDefOrRef> const& type, generic_arg_stack const& genericArgs, format_flags flags = format_flags::none);
inline void write_type_cpp(writer& w, xlang::meta::reader::TypeSig const& type, generic_arg_stack const& genericArgs, format_flags flags = format_flags::none);
inline void write_type_cpp(writer& w, xlang::meta::reader::GenericTypeInstSig const& type, generic_arg_stack const& genericArgs, format_flags flags = format_flags::none);

inline void write_type_mangled(writer& w, xlang::meta::reader::TypeDef const& type, generic_arg_stack const& genericArgs, format_flags flags = format_flags::none);
inline void write_type_mangled(writer& w, xlang::meta::reader::TypeRef const& ref, generic_arg_stack const& genericArgs, format_flags flags = format_flags::none);
inline void write_type_mangled(writer& w, xlang::meta::reader::ElementType type, generic_arg_stack const& genericArgs, format_flags flags = format_flags::none);
inline void write_type_mangled(writer& w, xlang::meta::reader::coded_index<xlang::meta::reader::TypeDefOrRef> const& type, generic_arg_stack const& genericArgs, format_flags flags = format_flags::none);
inline void write_type_mangled(writer& w, xlang::meta::reader::TypeSig const& type, generic_arg_stack const& genericArgs, format_flags flags = format_flags::none);
inline void write_type_mangled(writer& w, xlang::meta::reader::GenericTypeInstSig const& type, generic_arg_stack const& genericArgs, format_flags flags = format_flags::none);

inline void write_type_signature(writer& w, xlang::meta::reader::TypeDef const& type, generic_arg_stack const& genericArgs, format_flags flags = format_flags::none);
inline void write_type_signature(writer& w, xlang::meta::reader::TypeRef const& ref, generic_arg_stack const& genericArgs, format_flags flags = format_flags::none);
inline void write_type_signature(writer& w, xlang::meta::reader::ElementType type, generic_arg_stack const& genericArgs, format_flags flags = format_flags::none);
inline void write_type_signature(writer& w, xlang::meta::reader::coded_index<xlang::meta::reader::TypeDefOrRef> const& type, generic_arg_stack const& genericArgs, format_flags flags = format_flags::none);
inline void write_type_signature(writer& w, xlang::meta::reader::TypeSig const& type, generic_arg_stack const& genericArgs, format_flags flags = format_flags::none);
inline void write_type_signature(writer& w, xlang::meta::reader::GenericTypeInstSig const& type, generic_arg_stack const& genericArgs, format_flags flags = format_flags::none);

inline void write_type_guid(writer& w, xlang::meta::reader::TypeDef const& type);
inline void write_generic_type_guid(writer& w, xlang::meta::reader::GenericTypeInstSig const& type, generic_arg_stack const& genericArgs);

// Functions specialized for use with bind<...>(...)
inline void write_typedef_clr(
    writer& w,
    xlang::meta::reader::TypeDef const& type,
    generic_arg_stack const& genericArgs,
    format_flags flags = format_flags::none)
{
    write_type_clr(w, type, genericArgs, flags);
}

inline void write_generictype_clr(
    writer& w,
    xlang::meta::reader::GenericTypeInstSig const& type,
    generic_arg_stack const& genericArgs,
    format_flags flags = format_flags::none)
{
    return write_type_clr(w, type, genericArgs, flags);
}

inline void write_typedef_cpp(
    writer& w,
    xlang::meta::reader::TypeDef const& type,
    generic_arg_stack const& genericArgs,
    format_flags flags = format_flags::none)
{
    write_type_cpp(w, type, genericArgs, flags);
}

inline void write_typesig_cpp(
    writer& w,
    xlang::meta::reader::TypeSig const& type,
    generic_arg_stack const& genericArgs,
    format_flags flags = format_flags::none)
{
    return write_type_cpp(w, type, genericArgs, flags);
}

inline void write_generictype_cpp(
    writer& w,
    xlang::meta::reader::GenericTypeInstSig const& type,
    generic_arg_stack const& genericArgs,
    format_flags flags = format_flags::none)
{
    return write_type_cpp(w, type, genericArgs, flags);
}

inline void write_typedef_mangled(
    writer& w,
    xlang::meta::reader::TypeDef const& type,
    generic_arg_stack const& genericArgs,
    format_flags flags = format_flags::none)
{
    return write_type_mangled(w, type, genericArgs, flags);
}

inline void write_generictype_mangled(
    writer& w,
    xlang::meta::reader::GenericTypeInstSig const& type,
    generic_arg_stack const& genericArgs,
    format_flags flags = format_flags::none)
{
    return write_type_mangled(w, type, genericArgs, flags);
}



inline void write_type_clr(
    writer& w,
    xlang::meta::reader::TypeDef const& type,
    generic_arg_stack const& /*genericArgs*/,
    format_flags flags)
{
    if (flags_set(flags, format_flags::ignore_namespace))
    {
        w.write(type.TypeName());
    }
    else
    {
        w.write("%.%", type.TypeNamespace(), type.TypeName());
    }
}

inline void write_type_clr(
    writer& w,
    xlang::meta::reader::TypeRef const& ref,
    generic_arg_stack const& genericArgs,
    format_flags flags)
{
    if (ref.TypeNamespace() == system_namespace)
    {
        if (ref.TypeName() == "Guid")
        {
            w.write("Guid");
        }
        else
        {
            XLANG_ASSERT(false);
        }
    }
    else
    {
        write_type_clr(w, find_required(ref), genericArgs, flags);
    }
}

inline void write_type_clr(
    writer& w,
    xlang::meta::reader::ElementType type,
    generic_arg_stack const& /*genericArgs*/,
    format_flags /*flags*/)
{
    using namespace std::literals;
    using namespace xlang::meta::reader;

    auto str = [&]()
    {
        switch (type)
        {
        case ElementType::Boolean: return "Boolean"sv;
        case ElementType::Char: return "Char16"sv;
        case ElementType::U1: return "UInt8"sv;
        case ElementType::I2: return "Int16"sv;
        case ElementType::U2: return "UInt16"sv;
        case ElementType::I4: return "Int32"sv;
        case ElementType::U4: return "UInt32"sv;
        case ElementType::I8: return "Int64"sv;
        case ElementType::U8: return "UInt64"sv;
        case ElementType::R4: return "Single"sv;
        case ElementType::R8: return "Double"sv;
        case ElementType::String: return "String"sv;
        case ElementType::Object: return "Object"sv;
        default:
            XLANG_ASSERT(false);
            return ""sv;
        }
    }();
    w.write(str);
}

inline void write_type_clr(
    writer& w,
    xlang::meta::reader::coded_index<xlang::meta::reader::TypeDefOrRef> const& type,
    generic_arg_stack const& genericArgs,
    format_flags flags)
{
    using namespace xlang::meta::reader;
    visit(type, [&](auto const& t)
        {
            write_type_clr(w, t, genericArgs, flags);
        });
}

inline void write_type_clr(
    writer& w,
    xlang::meta::reader::TypeSig const& type,
    generic_arg_stack const& genericArgs,
    format_flags flags)
{
    using namespace xlang::meta::reader;
    xlang::visit(type.Type(),
        [&](GenericTypeIndex const& typeIndex)
        {
            auto [sig, newStack] = genericArgs.lookup(typeIndex.index);
            write_type_clr(w, sig, newStack, flags);
        },
        [&](auto const& t)
        {
            write_type_clr(w, t, genericArgs, flags);
        });
}

inline void write_type_clr(
    writer& w,
    xlang::meta::reader::GenericTypeInstSig const& type,
    generic_arg_stack const& genericArgs,
    format_flags flags)
{
    using namespace std::literals;

    write_type_clr(w, type.GenericType(), genericArgs, flags);
    w.write('<');

    std::string_view prefix;
    auto const genericFlags = clear_flags(flags, format_flags::ignore_namespace) | format_flags::generic_param;
    for (auto const& param : type.GenericArgs())
    {
        w.write(prefix);
        write_type_clr(w, param, genericArgs, genericFlags);
        prefix = ", "sv;
    }

    w.write('>');
}



inline void write_type_cpp(
    writer& w,
    xlang::meta::reader::TypeDef const& type,
    generic_arg_stack const& genericArgs,
    format_flags flags)
{
    using namespace xlang::meta::reader;
    using namespace xlang::text;

    bool const genericParam = flags_set(flags, format_flags::generic_param);
    bool const functionParam = flags_set(flags, format_flags::function_param);
    bool const ignoreNamespace = flags_set(flags, format_flags::ignore_namespace);
    auto typeCategory = get_category(type);

    // Types used as either function or generic parameters get mangled, so we shouldn't ever make it this far
    XLANG_ASSERT(!genericParam || (distance(type.GenericParam()) == 0));
    XLANG_ASSERT(!functionParam || (distance(type.GenericParam()) == 0));

    // Function parameters must be ABI types
    if (functionParam && (typeCategory == category::class_type))
    {
        XLANG_ASSERT(!genericParam); // We should be mangling these names
        write_type_cpp(w, default_interface(type), genericArgs, flags);
        return;
    }

    switch (typeCategory)
    {
    case category::struct_type:
        if (genericParam || functionParam)
        {
            w.write("struct ");
        }
        break;

    case category::enum_type:
        if (genericParam)
        {
            w.write(w.config().enum_class ? "MIDL_ENUM " : "enum ");
        }
        break;
    }

    if (!ignoreNamespace)
    {
        write_namespace_open(w, type.TypeNamespace());
    }

    // C++ names don't carry the tick mark
    auto name = type.TypeName();
    name = name.substr(0, name.find('`'));
    w.write("%%", type_prefix(typeCategory), name);

    if (genericParam || functionParam)
    {
        switch (typeCategory)
        {
        case category::interface_type:
        case category::class_type:
        case category::delegate_type:
            w.write('*');
            break;
        }
    }

    if (!ignoreNamespace)
    {
        write_namespace_close(w);
    }
}

inline void write_type_cpp(
    writer& w,
    xlang::meta::reader::TypeRef const& ref,
    generic_arg_stack const& genericArgs,
    format_flags flags)
{
    if (ref.TypeNamespace() == system_namespace)
    {
        if (ref.TypeName() == "Guid")
        {
            w.write("GUID");
        }
        else
        {
            XLANG_ASSERT(false);
        }
    }
    else
    {
        write_type_cpp(w, find_required(ref), genericArgs, flags);
    }
}

inline void write_type_cpp(
    writer& w,
    xlang::meta::reader::ElementType type,
    generic_arg_stack const& /*genericArgs*/,
    format_flags flags)
{
    using namespace std::literals;
    using namespace xlang::meta::reader;

    auto str = [&]()
    {
    switch (type)
    {
        case ElementType::Void: return "void"sv;
        case ElementType::Boolean: return flags_set(flags, format_flags::generic_param) ? "bool"sv : "::boolean"sv;
        case ElementType::Char: return "wchar_t"sv;
        case ElementType::U1: return "::byte"sv;
        case ElementType::I2: return "short"sv;
        case ElementType::U2: return flags_set(flags, format_flags::generic_param) ? "UINT16"sv : "unsigned short"sv;
        case ElementType::I4: return "int"sv;
        case ElementType::U4: return flags_set(flags, format_flags::generic_param) ? "UINT32"sv : "unsigned int"sv;
        case ElementType::I8: return "__int64"sv;
        case ElementType::U8: return flags_set(flags, format_flags::generic_param) ? "UINT64"sv : "unsigned __int64"sv;
        case ElementType::R4: return "float"sv;
        case ElementType::R8: return "double"sv;
        case ElementType::String: return "HSTRING"sv;
        case ElementType::Object: return "IInspectable*"sv;
        default:
            XLANG_ASSERT(false);
            return ""sv;
        }
    }();
    w.write(str);
}

inline void write_type_cpp(
    writer& w,
    xlang::meta::reader::coded_index<xlang::meta::reader::TypeDefOrRef> const& type,
    generic_arg_stack const& genericArgs,
    format_flags flags)
{
    using namespace xlang::meta::reader;
    visit(type, [&](auto const& t)
        {
            write_type_cpp(w, t, genericArgs, flags);
        });
}

inline void write_type_cpp(
    writer& w,
    xlang::meta::reader::TypeSig const& type,
    generic_arg_stack const& genericArgs,
    format_flags flags)
{
    using namespace xlang::meta::reader;
    xlang::visit(type.Type(),
        [&](GenericTypeIndex const& typeIndex)
        {
            auto [sig, newStack] = genericArgs.lookup(typeIndex.index);
            write_type_cpp(w, sig, newStack, flags);
        },
        [&](auto const& t)
        {
            write_type_cpp(w, t, genericArgs, flags);
        });
}

inline void write_type_cpp(
    writer& w,
    xlang::meta::reader::GenericTypeInstSig const& type,
    generic_arg_stack const& genericArgs,
    format_flags flags)
{
    using namespace std::literals;

    if (any_flag_set(flags, format_flags::generic_or_function_param))
    {
        // Generics get mangled when supplied as function or template arguments
        write_type_mangled(w, type, genericArgs, flags);
        w.write('*');
    }
    else
    {
        write_type_cpp(w, type.GenericType(), genericArgs, flags);
        w.write('<');

        std::string_view prefix;
        auto const genericFlags = clear_flags(flags, format_flags::ignore_namespace) | format_flags::generic_param;
        for (auto const& param : type.GenericArgs())
        {
            w.write(prefix);
            write_type_cpp(w, param, genericArgs, genericFlags);
            prefix = ", ";
        }

        w.write('>');
    }
}



inline void write_type_mangled(
    writer& w,
    xlang::meta::reader::TypeDef const& type,
    generic_arg_stack const& /*genericArgs*/,
    format_flags flags)
{
    using namespace xlang::meta::reader;
    using namespace xlang::text;

    auto writeName = [&](std::string_view name)
    {
        for (auto const ch : name)
        {
            if (ch == '.')
            {
                w.write(flags_set(flags, format_flags::generic_param) ? "__C" : "_C");
            }
            else if (ch == '_')
            {
                w.write(flags_set(flags, format_flags::generic_param) ? "__z" : "__");
            }
            else if (ch == '`')
            {
                w.write('_');
            }
            else
            {
                w.write(ch);
            }
        }
    };

    // Generic types get "__F" as a prefix instead of the namespace
    bool const isGenericType = distance(type.GenericParam()) != 0;
    if (isGenericType)
    {
        w.write("__F");
    }
    else
    {
        if (flags_clear(flags, format_flags::generic_param))
        {
            w.write("__x_");
            if (w.config().ns_prefix_state == ns_prefix::always)
            {
                w.write("ABI_C");
            }
        }

        writeName(type.TypeNamespace());
        w.write(flags_set(flags, format_flags::generic_param) ? "__C" : "_C");
    }

    w.write(type_prefix(get_category(type)));
    writeName(type.TypeName());
}

inline void write_type_mangled(
    writer& w,
    xlang::meta::reader::TypeRef const& ref,
    generic_arg_stack const& genericArgs,
    format_flags flags)
{
    if (ref.TypeNamespace() == system_namespace)
    {
        if (ref.TypeName() == "Guid")
        {
            w.write("GUID");
        }
        else
        {
            XLANG_ASSERT(false);
        }
    }
    else
    {
        write_type_mangled(w, find_required(ref), genericArgs, flags);
    }
}

inline void write_type_mangled(
    writer& w,
    xlang::meta::reader::ElementType type,
    generic_arg_stack const& /*genericArgs*/,
    format_flags /*flags*/)
{
    using namespace std::literals;
    using namespace xlang::meta::reader;

    auto str = [&]()
    {
        switch (type)
        {
        case ElementType::Boolean: return "boolean"sv;
        case ElementType::Char: return "wchar__zt"sv;
        case ElementType::U1: return "byte"sv;
        case ElementType::I2: return "short"sv;
        case ElementType::U2: return "UINT16"sv;
        case ElementType::I4: return  "int"sv;
        case ElementType::U4: return "UINT32"sv;
        case ElementType::I8: return "__z__zint64"sv;
        case ElementType::U8: return "UINT64"sv;
        case ElementType::R4: return "float"sv;
        case ElementType::R8: return "double"sv;
        case ElementType::String: return "HSTRING"sv;
        case ElementType::Object: return "IInspectable"sv;
        default:
            XLANG_ASSERT(false);
            return ""sv;
        }
    }();
    w.write(str);
}

inline void write_type_mangled(
    writer& w,
    xlang::meta::reader::coded_index<xlang::meta::reader::TypeDefOrRef> const& type,
    generic_arg_stack const& genericArgs,
    format_flags flags)
{
    using namespace xlang::meta::reader;
    visit(type, [&](auto const& t)
        {
            write_type_mangled(w, t, genericArgs, flags);
        });
}

inline void write_type_mangled(
    writer& w,
    xlang::meta::reader::TypeSig const& type,
    generic_arg_stack const& genericArgs,
    format_flags flags)
{
    using namespace xlang::meta::reader;
    xlang::visit(type.Type(),
        [&](GenericTypeIndex const& typeIndex)
        {
            auto [sig, newStack] = genericArgs.lookup(typeIndex.index);
            write_type_mangled(w, sig, newStack, flags);
        },
        [&](auto const& t)
        {
            write_type_mangled(w, t, genericArgs, flags);
        });
}

inline void write_type_mangled(
    writer& w,
    xlang::meta::reader::GenericTypeInstSig const& type,
    generic_arg_stack const& genericArgs,
    format_flags flags)
{
    using namespace std::literals;

    write_type_mangled(w, type.GenericType(), genericArgs, flags);
    w.write('_');

    std::string_view prefix;
    auto const genericFlags = clear_flags(flags, format_flags::ignore_namespace) | format_flags::generic_param;
    for (auto const& param : type.GenericArgs())
    {
        w.write(prefix);
        write_type_mangled(w, param, genericArgs, genericFlags);
        prefix = "_"sv;
    }
}




inline void write_type_signature(
    writer& w,
    xlang::meta::reader::TypeDef const& type,
    generic_arg_stack const& genericArgs,
    format_flags flags)
{
    using namespace xlang::meta::reader;
    using namespace xlang::text;

    auto const typeCategory = get_category(type);
    bool const writeCategory = flags_set(flags, format_flags::generic_param) && (typeCategory != category::interface_type);
    if (writeCategory)
    {
        switch (typeCategory)
        {
        case category::class_type:
            w.write("rc(");
            break;

        case category::enum_type:
            w.write("enum(");
            break;

        case category::struct_type:
            w.write("struct(");
            break;

        case category::delegate_type:
            w.write("delegate(");
            break;

        default:
            XLANG_ASSERT(false);
        }
    }

    switch (get_category(type))
    {
    case category::interface_type:
        w.write("{%}", bind<write_type_guid>(type));
        break;

    case category::class_type:
        w.write("%.%;{%}", type.TypeNamespace(), type.TypeName(), bind<write_type_guid>(type));
        break;

    case category::enum_type:
        w.write("%.%;", type.TypeNamespace(), type.TypeName());
        write_type_signature(w, type.FieldList().first.Signature().Type(), genericArgs, flags);
        break;

    case category::struct_type:
        w.write("%.%", type.TypeNamespace(), type.TypeName());
        for (auto const& field : type.FieldList())
        {
            w.write(";");
            write_type_signature(w, field.Signature().Type(), genericArgs, flags);
        }
        break;

    case category::delegate_type:
        w.write("{%}", bind<write_type_guid>(type));
        break;
    }

    if (writeCategory)
    {
        w.write(")");
    }
}

inline void write_type_signature(
    writer& w,
    xlang::meta::reader::TypeRef const& ref,
    generic_arg_stack const& genericArgs,
    format_flags flags)
{
    if (ref.TypeNamespace() == system_namespace)
    {
        if (ref.TypeName() == "Guid")
        {
            w.write("g16");
        }
        else
        {
            XLANG_ASSERT(false);
        }
    }
    else
    {
        write_type_signature(w, find_required(ref), genericArgs, flags);
    }
}

inline void write_type_signature(
    writer& w,
    xlang::meta::reader::ElementType type,
    generic_arg_stack const& /*genericArgs*/,
    format_flags /*flags*/)
{
    using namespace std::literals;
    using namespace xlang::meta::reader;

    auto str = [&]()
    {
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
        default:
            XLANG_ASSERT(false);
            return ""sv;
        }
    }();
    w.write(str);
}

inline void write_type_signature(
    writer& w,
    xlang::meta::reader::coded_index<xlang::meta::reader::TypeDefOrRef> const& type,
    generic_arg_stack const& genericArgs,
    format_flags flags)
{
    using namespace xlang::meta::reader;
    visit(type, [&](auto const& t)
        {
            write_type_signature(w, t, genericArgs, flags);
        });
}

inline void write_type_signature(
    writer& w,
    xlang::meta::reader::TypeSig const& type,
    generic_arg_stack const& genericArgs,
    format_flags flags)
{
    using namespace xlang::meta::reader;
    xlang::visit(type.Type(),
        [&](GenericTypeIndex const& typeIndex)
        {
            auto [sig, newStack] = genericArgs.lookup(typeIndex.index);
            write_type_signature(w, sig, newStack, flags);
        },
        [&](auto const& t)
        {
            write_type_signature(w, t, genericArgs, flags);
        });
}

inline void write_type_signature(
    writer& w,
    xlang::meta::reader::GenericTypeInstSig const& type,
    generic_arg_stack const& genericArgs,
    format_flags flags)
{
    using namespace std::literals;

    w.write("pinterface(");
    write_type_signature(w, type.GenericType(), genericArgs, clear_flags(flags, format_flags::generic_param));

    auto const genericFlags = flags | format_flags::generic_param;
    for (auto const& param : type.GenericArgs())
    {
        w.write(";");
        write_type_signature(w, param, genericArgs, genericFlags);
    }
    w.write(')');
}



inline void write_type_guid(writer& w, xlang::meta::reader::TypeDef const& type)
{
    using namespace xlang::meta::reader;

    if (get_category(type) == category::class_type)
    {
        // Classes don't have interface guids; we instead want to use the class' default interface
        ::visit(default_interface(type), xlang::visit_overload{
            [&](GenericTypeInstSig const& sig)
            {
                write_generic_type_guid(w, sig, generic_arg_stack::empty()); // TODO: generic_arg_stack
            },
            [&](auto const& defOrRef)
            {
                write_type_guid(w, find_required(defOrRef));
            }});
        return;
    }

    auto attr = get_attribute(type, metadata_namespace, guid_attribute);
    if (!attr)
    {
        xlang::throw_invalid("'Windows.Foundation.Metadata.GuidAttribute' attribute for type '", type.TypeNamespace(), ".", type.TypeName(), "' not found");
    }

    auto value = attr.Value();
    auto const& args = value.FixedArgs();
    w.write_printf("%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
        std::get<uint32_t>(std::get<ElemSig>(args[0].value).value),
        std::get<uint16_t>(std::get<ElemSig>(args[1].value).value),
        std::get<uint16_t>(std::get<ElemSig>(args[2].value).value),
        std::get<uint8_t>(std::get<ElemSig>(args[3].value).value),
        std::get<uint8_t>(std::get<ElemSig>(args[4].value).value),
        std::get<uint8_t>(std::get<ElemSig>(args[5].value).value),
        std::get<uint8_t>(std::get<ElemSig>(args[6].value).value),
        std::get<uint8_t>(std::get<ElemSig>(args[7].value).value),
        std::get<uint8_t>(std::get<ElemSig>(args[8].value).value),
        std::get<uint8_t>(std::get<ElemSig>(args[9].value).value),
        std::get<uint8_t>(std::get<ElemSig>(args[10].value).value));
}

inline void write_generic_type_guid(writer& w, xlang::meta::reader::GenericTypeInstSig const& type, generic_arg_stack const& genericArgs)
{
    using namespace xlang::text;

    // For generic types, we need to take the SHA1 hash of the type's signature, and use that for the GUID
    sha1 hash;

    // NOTE: The type namespace GUID needs to get processed as big endian values, hence the byte array
    static constexpr std::uint8_t namespaceGuidBytes[] =
    {
        0x11, 0xf4, 0x7a, 0xd5,
        0x7b, 0x73,
        0x42, 0xc0,
        0xab, 0xae, 0x87, 0x8b, 0x1e, 0x16, 0xad, 0xee
    };
    hash.append(namespaceGuidBytes, std::size(namespaceGuidBytes));

    auto const strSignature = w.write_temp("%", [&](writer& w) { write_type_signature(w, type, genericArgs, format_flags::none); });
    hash.append(reinterpret_cast<const std::uint8_t*>(strSignature.c_str()), strSignature.length());

    // Hash gives us 20 bytes, but GUID only takes 16; discard the 4 "low" bytes. We also need to encode a version
    // number in the high nibble of 'Data3' as well as set the variant field
    auto const hashValues = hash.finalize();
    w.write_printf("%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
        hashValues[0], hashValues[1], hashValues[2], hashValues[3],
        hashValues[4], hashValues[5],
        ((hashValues[6] & 0x0F) | 0x50), hashValues[7],
        ((hashValues[8] & 0x3F) | 0x80), hashValues[9],
        hashValues[10], hashValues[11], hashValues[12], hashValues[13], hashValues[14], hashValues[15]);
}

template <typename LogicT, typename AbiT>
inline void write_aggregate_type(writer& w, LogicT&& logical, AbiT&& abi)
{
    using namespace xlang::text;
    w.write("%%%<%, %>",
        bind<write_namespace_open>(internal_namespace),
        "AggregateType",
        bind<write_namespace_close>(),
        logical,
        abi);
}

inline void write_generic_impl_param(
    writer& w,
    xlang::meta::reader::TypeSig const& type,
    generic_arg_stack const& genericArgs)
{
    using namespace xlang::meta::reader;
    using namespace xlang::text;

    xlang::visit(type.Type(),
        [&](coded_index<TypeDefOrRef> const& t)
        {
            visit(t, xlang::visit_overload{
                [&](GenericTypeInstSig const& sig)
                {
                    // Generic types never get aggregated
                    write_type_cpp(w, sig, genericArgs, format_flags::generic_param);
                },
                [&](auto const& defOrRef)
                {
                    if (defOrRef.TypeNamespace() == system_namespace)
                    {
                        if (defOrRef.TypeName() == "Guid")
                        {
                            w.write("GUID");
                        }
                        else
                        {
                            XLANG_ASSERT(false);
                        }
                    }
                    else
                    {
                        auto&& def = find_required(defOrRef);
                        switch (get_category(def))
                        {
                        case category::class_type:
                        {
                            // Only class types get aggregated
                            auto iface = default_interface(def);
                            write_aggregate_type(w,
                                bind<write_typedef_cpp>(def, genericArgs, format_flags::generic_param),
                                [&](auto& w) { write_type_cpp(w, iface, genericArgs, format_flags::generic_param); });
                        }   break;

                        case category::interface_type:
                        case category::enum_type:
                        case category::struct_type:
                        case category::delegate_type:
                            write_type_cpp(w, def, genericArgs, format_flags::generic_param);
                            break;
                        }
                    }
                }});
        },
        [&](GenericTypeInstSig const& t)
        {
            // Generic types never get aggregated
            write_type_cpp(w, t, genericArgs, format_flags::generic_param);
        },
        [&](ElementType t)
        {
            // The only element type that gets mapped is 'bool', which becomes 'boolean'
            if (t == ElementType::Boolean)
            {
                write_aggregate_type(w, "bool", "boolean");
            }
            else
            {
                write_type_cpp(w, t, genericArgs, format_flags::generic_param);
            }
        },
        [&](GenericTypeIndex t)
        {
            auto [sig, newStack] = genericArgs.lookup(t.index);
            write_generic_impl_param(w, sig, newStack);
        });
}
