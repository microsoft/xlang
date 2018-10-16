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

enum class type_format
{
    // There's really four ways we want to format types:
    clr = 0,        // "CLR" form. E.g. "Foo.Bar.IPair`2<Foo.Bar.Key, Foo.Bar.Value>"
    cpp = 1,        // "C++" form. E.g. "Foo::Bar::IPair<Foo::Bar::Key*, Foo::Bar::Value*>"
    mangled = 2,    // "Mangled"/"C" form. E.g. "__FIPair_2_Foo__CBar__CKey_Foo__CBar__CValue"
    signature = 3,  // E.g. "pinterface({guid...};rc(Foo.Bar.Key;{guid...});struct(Foo.Bar.Value;i4))"
    form_mask = 0x0003,

    // We also want to controll how the type gets formatted
    generic_param = 0x0004,     // Modifies formatting in a set of circumstances (e.g. add '*' when needed, etc.)
    ignore_namespace = 0x0008,  // Only writes the type name. Only valid in a small set of situations
};
inline constexpr type_format operator&(type_format lhs, type_format rhs) { return static_cast<type_format>(static_cast<int>(lhs) & static_cast<int>(rhs)); }
inline constexpr type_format operator|(type_format lhs, type_format rhs) { return static_cast<type_format>(static_cast<int>(lhs) | static_cast<int>(rhs)); }
inline constexpr type_format operator~(type_format value) { return static_cast<type_format>(~static_cast<int>(value)); }

inline constexpr bool format_as(type_format format, type_format form)
{
    return (format & type_format::form_mask) == form;
}

inline constexpr bool options_set(type_format format, type_format options)
{
    return (format & options) == options;
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

inline void write_generic_type_guid(
    writer& w,
    xlang::meta::reader::GenericTypeInstSig const& type,
    generic_arg_stack const& genericArgs = generic_arg_stack::empty());

inline void write_type_guid(writer& w, xlang::meta::reader::TypeDef const& type)
{
    using namespace xlang::meta::reader;

    if (get_category(type) == category::class_type)
    {
        // Classes don't have interface guids; we instead want to use the class' default interface
        ::visit(default_interface(type), xlang::visit_overload{
            [&](GenericTypeInstSig const& sig)
            {
                write_generic_type_guid(w, sig);
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

template <type_format Format>
inline void write_type(writer& w, xlang::meta::reader::TypeSig const& type, generic_arg_stack const& genericArgs);

template <type_format Format>
inline void write_type(writer& w, xlang::meta::reader::TypeDef const& type, generic_arg_stack const& genericArgs)
{
    using namespace xlang::meta::reader;
    using namespace xlang::text;

    auto ns = type.TypeNamespace();
    auto name = type.TypeName();
    [[maybe_unused]] auto const isGenericType = xlang::meta::reader::distance(type.GenericParam()) != 0;
    [[maybe_unused]] auto const typeCategory = get_category(type);
    if constexpr (format_as(Format, type_format::clr))
    {
        w.write("%.%", ns, name);
    }
    else if constexpr (format_as(Format, type_format::signature))
    {
        switch (typeCategory)
        {
        case category::interface_type:
            w.write("{%}", bind<write_type_guid>(type));
            break;

        case category::class_type:
            w.write("rc(%.%;{%})", type.TypeNamespace(), type.TypeName(), bind<write_type_guid>(type));
            break;

        case category::enum_type:
            w.write("enum(%.%;", type.TypeNamespace(), type.TypeName());
            write_type<Format>(w, type.FieldList().first.Signature().Type(), genericArgs);
            w.write(")");
            break;

        case category::struct_type:
            w.write("struct(%.%", type.TypeNamespace(), type.TypeName());
            for (auto const& field : type.FieldList())
            {
                w.write(";");
                write_type<Format>(w, field.Signature().Type(), genericArgs);
            }
            w.write(")");
            break;

        case category::delegate_type:
            if constexpr (options_set(Format, type_format::generic_param))
            {
                w.write("delegate({%})", bind<write_type_guid>(type));
            }
            else
            {
                w.write("{%}", bind<write_type_guid>(type));
            }
            break;
        }
    }
    else if constexpr (format_as(Format, type_format::cpp))
    {
        XLANG_ASSERT(!options_set(Format, type_format::generic_param) || !isGenericType);

        if constexpr (options_set(Format, type_format::generic_param))
        {
            switch (typeCategory)
            {
            case category::struct_type:
                w.write("struct ");
                break;

            case category::enum_type:
                w.write(w.config().enum_class ? "MIDL_ENUM " : "enum ");
                break;
            }
        }

        if constexpr (!options_set(Format, type_format::ignore_namespace))
        {
            write_namespace_open(w, ns);
        }

        // C++ names don't carry the tick mark
        name = name.substr(0, name.find('`'));
        w.write("%%", type_prefix(typeCategory), name);

        if constexpr (options_set(Format, type_format::generic_param))
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

        if constexpr (!options_set(Format, type_format::ignore_namespace))
        {
            write_namespace_close(w);
        }
    }
    else if constexpr (format_as(Format, type_format::mangled))
    {
        auto writeName = [&](std::string_view name)
        {
            for (auto const ch : name)
            {
                if (ch == '.')
                {
                    w.write(options_set(Format, type_format::generic_param) ? "__C" : "_C");
                }
                else if (ch == '_')
                {
                    w.write(options_set(Format, type_format::generic_param) ? "__z" : "__");
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

        // Generics get "__F" as a prefix instead of the namespace. This also bypasses any ABI prefix
        if (isGenericType)
        {
            w.write("__F");
        }
        else if constexpr (!options_set(Format, type_format::generic_param))
        {
            w.write("__x_");
            if (w.config().ns_prefix_state == ns_prefix::always)
            {
                w.write("ABI_C");
            }
        }
        if (!isGenericType)
        {
            writeName(ns);
            w.write(options_set(Format, type_format::generic_param) ? "__C" : "_C");
        }

        w.write(type_prefix(typeCategory));
        writeName(name);
    }
    else
    {
        static_assert(false, "Invalid formatting option");
    }
}

template <type_format Format>
inline void write_type(writer& w, xlang::meta::reader::TypeRef const& ref, generic_arg_stack const& genericArgs)
{
    write_type<Format>(w, find_required(ref), genericArgs);
}

template <type_format Format>
inline void write_type(writer& w, xlang::meta::reader::ElementType type, generic_arg_stack const& /*genericArgs*/)
{
    using namespace std::literals;

    std::string_view str;
    switch (type)
    {
    case ElementType::Void:
        XLANG_ASSERT(!format_as(Format, type_format::clr) && !format_as(Format, type_format::signature));
        str = "void"sv;
        break;

    case ElementType::Boolean:
        str = format_as(Format, type_format::clr) ? "Boolean"sv :
              format_as(Format, type_format::signature) ? "b1"sv :
              options_set(Format, type_format::generic_param) ? "bool" : "boolean"sv;
        break;

    case ElementType::Char:
        str = format_as(Format, type_format::clr) ? "Char16"sv :
              format_as(Format, type_format::signature) ? "c2"sv :
              format_as(Format, type_format::cpp) ? "wchar_t"sv : "wchar__zt"sv;
        break;

    case ElementType::I1:
        XLANG_ASSERT(false); // TODO? This type seems to be a lie
        break;

    case ElementType::U1:
        str = format_as(Format, type_format::clr) ? "UInt8"sv :
              format_as(Format, type_format::signature) ? "u1"sv :
              format_as(Format, type_format::cpp) ? "::byte"sv : "byte"sv;
        break;

    case ElementType::I2:
        str = format_as(Format, type_format::clr) ? "Int16"sv :
              format_as(Format, type_format::signature) ? "i2"sv : "short"sv;
        break;

    case ElementType::U2:
        str = format_as(Format, type_format::clr) ? "UInt16"sv :
              format_as(Format, type_format::signature) ? "u2"sv : "UINT16"sv;
        break;

    case ElementType::I4:
        str = format_as(Format, type_format::clr) ? "Int32"sv :
              format_as(Format, type_format::signature) ? "i4"sv : "int"sv;
        break;

    case ElementType::U4:
        str = format_as(Format, type_format::clr) ? "UInt32"sv :
              format_as(Format, type_format::signature) ? "u4"sv : "UINT32"sv;
        break;

    case ElementType::I8:
        str = format_as(Format, type_format::clr) ? "Int64"sv :
              format_as(Format, type_format::signature) ? "i8"sv :
              format_as(Format, type_format::cpp) ? "__int64"sv : "__z__zint64"sv;
        break;

    case ElementType::U8:
        str = format_as(Format, type_format::clr) ? "UInt64"sv :
              format_as(Format, type_format::signature) ? "u8"sv : "UINT64"sv;
        break;

    case ElementType::R4:
        str = format_as(Format, type_format::clr) ? "Single"sv :
              format_as(Format, type_format::signature) ? "f4"sv : "float"sv;
        break;

    case ElementType::R8:
        str = format_as(Format, type_format::clr) ? "Double"sv :
              format_as(Format, type_format::signature) ? "f8"sv : "double"sv;
        break;

    case ElementType::String:
        str = format_as(Format, type_format::clr) ? "String"sv :
              format_as(Format, type_format::signature) ? "string"sv : "HSTRING"sv;
        break;

    case ElementType::Object:
        str = format_as(Format, type_format::clr) ? "Object"sv :
              format_as(Format, type_format::signature) ? "cinterface(IInspectable)"sv :
              format_as(Format, type_format::cpp) ? "IInspectable*"sv : "IInspectable"sv;
        break;

    default:
        XLANG_ASSERT(false);
    }

    w.write(str);
}

template <type_format Format>
inline void write_type(writer& w, xlang::meta::reader::coded_index<xlang::meta::reader::TypeDefOrRef> const& type, generic_arg_stack const& genericArgs)
{
    using namespace xlang::meta::reader;
    visit(type, [&](auto const& t)
        {
            write_type<Format>(w, t, genericArgs);
        });
}

template <type_format Format>
inline void write_type(writer& w, xlang::meta::reader::TypeSig const& type, generic_arg_stack const& genericArgs)
{
    xlang::visit(type.Type(),
        [&](xlang::meta::reader::GenericTypeIndex const& typeIndex)
        {
            auto [sig, newStack] = genericArgs.lookup(typeIndex.index);
            write_type<Format>(w, sig, newStack);
        },
        [&](auto const& t)
        {
            write_type<Format>(w, t, genericArgs);
        });
}

template <type_format Format>
inline void write_type(
    writer& w,
    xlang::meta::reader::GenericTypeInstSig const& type,
    generic_arg_stack const& genericArgs)
{
    using namespace std::literals;

    if constexpr (format_as(Format, type_format::cpp) && options_set(Format, type_format::generic_param))
    {
        // Generics get mangled when supplied as template arguments
        write_type<type_format::mangled>(w, type, genericArgs);
        w.write("*");
    }
    else if constexpr (format_as(Format, type_format::signature))
    {
        w.write("pinterface(");
        write_type<Format>(w, type.GenericType(), genericArgs);

        constexpr auto generic_format = Format | type_format::generic_param;
        for (auto const& param : type.GenericArgs())
        {
            w.write(";");
            write_type<generic_format>(w, param, genericArgs);
        }
        w.write(")");
    }
    else
    {
        write_type<Format>(w, type.GenericType(), genericArgs);
        w.write((format_as(Format, type_format::clr) || format_as(Format, type_format::cpp)) ? "<"sv : "_"sv);

        std::string_view prefix;
        constexpr auto generic_format = (Format & ~type_format::ignore_namespace) | type_format::generic_param;
        for (auto const& param : type.GenericArgs())
        {
            w.write(prefix);
            write_type<generic_format>(w, param, genericArgs);
            prefix = (format_as(Format, type_format::clr) || format_as(Format, type_format::cpp)) ? ", "sv : "_"sv;
        }

        w.write((format_as(Format, type_format::clr) || format_as(Format, type_format::cpp)) ? ">"sv : ""sv);
    }
}

template <type_format Format>
inline void write_type_def(
    writer& w,
    xlang::meta::reader::TypeDef const& type,
    generic_arg_stack const& genericArgs = generic_arg_stack::empty())
{
    write_type<Format>(w, type, genericArgs);
}

template <type_format Format>
inline void write_generic_type_inst_sig(
    writer& w,
    xlang::meta::reader::GenericTypeInstSig const& type,
    generic_arg_stack const& genericArgs = generic_arg_stack::empty())
{
    write_type<Format>(w, type, genericArgs);
}

inline void write_generic_type_guid(
    writer& w,
    xlang::meta::reader::GenericTypeInstSig const& type,
    generic_arg_stack const& genericArgs)
{
    using namespace xlang::text;

    // For generic types, we need to take the SHA1 hash of the type's signature, and use that for the GUID
    sha1 hash;

    // NOTE: The type namespace GUID needs to get processed as big endian values
    static constexpr std::uint8_t namespaceGuidBytes[] =
    {
        0x11, 0xf4, 0x7a, 0xd5,
        0x7b, 0x73,
        0x42, 0xc0,
        0xab, 0xae, 0x87, 0x8b, 0x1e, 0x16, 0xad, 0xee
    };
    hash.append(namespaceGuidBytes, std::size(namespaceGuidBytes));

    auto const strSignature = w.write_temp("%", bind<write_generic_type_inst_sig<type_format::signature>>(type, genericArgs));
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
