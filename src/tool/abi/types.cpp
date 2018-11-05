#include "pch.h"

#include "abi_writer.h"
#include "code_writers.h"
#include "types.h"

using namespace std::literals;
using namespace xlang::meta::reader;
using namespace xlang::text;

void enum_type::write_cpp_forward_declaration(writer& w) const
{
    w.push_namespace(clr_abi_namespace());
    auto enumStr = w.config().enum_class ? "MIDL_ENUM"sv : "enum"sv;
    auto typeStr = underlying_type() == ElementType::I4 ? "int"sv : "unsigned int"sv;
    w.write("%typedef % % : % %;\n", indent{}, enumStr, cpp_abi_name(), typeStr, cpp_abi_name());
    w.pop_namespace();
    w.write('\n');
}

void enum_type::write_cpp_generic_param_abi_type(writer& w) const
{
    write_cpp_abi_type(w);
}

void enum_type::write_cpp_abi_type(writer& w) const
{
    w.write("enum %", bind<write_cpp_fully_qualified_type>(clr_abi_namespace(), cpp_abi_name()));
}

void enum_type::write_cpp_definition(writer& w) const
{
    auto name = cpp_abi_name();

    // NOTE: MIDL writes 'Struct' for enums, so we will, too
    w.write(R"^-^(/*
 *
 * Struct %
 )^-^", m_clrFullName);

    if (auto contractInfo = contract_attributes(m_type))
    {
        w.write(R"^-^( *
 * Introduced to % in version %
)^-^", contractInfo->type_name, bind<write_contract_version>(contractInfo->version));
    }

    w.write(R"^-^( *
 */
)^-^");

    auto contractDepth = w.push_contract_guards(m_type);

    w.push_namespace(clr_abi_namespace());

    auto enumStr = w.config().enum_class ? "MIDL_ENUM"sv : "enum"sv;
    w.write("%%", indent{}, enumStr);
    if (auto info = is_deprecated(m_type))
    {
        w.write("\n");
        write_deprecation_message(w, *info);
        w.write("%", indent{});
    }
    else
    {
        w.write(' ');
    }

    auto typeStr = underlying_type() == ElementType::I4 ? "int"sv : "unsigned int"sv;
    w.write(R"^-^(% : %
%{
)^-^", name, typeStr, indent{});

    for (auto const& field : m_type.FieldList())
    {
        if (auto value = field.Constant())
        {
            auto fieldContractDepth = w.push_contract_guards(field);

            w.write("%", indent{ 1 });
            if (!w.config().enum_class)
            {
                w.write("%_", name);
            }
            w.write(field.Name());

            if (auto info = is_deprecated(field))
            {
                w.write("\n");
                write_deprecation_message(w, *info, 1, "DEPRECATEDENUMERATOR");
                w.write("%", indent{ 1 });
            }
            else
            {
                w.write(' ');
            }

            w.write("= %,\n", value);
            w.pop_contract_guards(fieldContractDepth);
        }
    }

    w.write("%};\n", indent{});

    if (is_flags_enum(m_type))
    {
        w.write("\n%DEFINE_ENUM_FLAG_OPERATORS(%)\n", indent{}, name);
    }

    if (w.config().enum_class)
    {
        w.write('\n');
        for (auto const& field : m_type.FieldList())
        {
            if (field.Constant())
            {
                w.write("%const % %_% = %::%;\n", indent{}, name, name, field.Name(), name, field.Name());
            }
        }
    }

    w.pop_namespace();
    w.pop_contract_guards(contractDepth);
    w.write('\n');
}

void struct_type::write_cpp_forward_declaration(writer& w) const
{
    w.push_namespace(clr_abi_namespace());
    w.write("%typedef struct % %;\n", indent{}, cpp_abi_name(), cpp_abi_name());
    w.pop_namespace();
    w.write('\n');
}

void struct_type::write_cpp_generic_param_abi_type(writer& w) const
{
    write_cpp_abi_type(w);
}

void struct_type::write_cpp_abi_type(writer& w) const
{
    w.write("struct %", bind<write_cpp_fully_qualified_type>(clr_abi_namespace(), cpp_abi_name()));
}

void struct_type::write_cpp_definition(writer& w) const
{
    w.write(R"^-^(/*
 *
 * Struct %
 )^-^", m_clrFullName);

    if (auto contractInfo = contract_attributes(m_type))
    {
        w.write(R"^-^( *
 * Introduced to % in version %
)^-^", contractInfo->type_name, bind<write_contract_version>(contractInfo->version));
    }

    w.write(R"^-^( *
 */
)^-^");

    auto contractDepth = w.push_contract_guards(m_type);
    w.push_namespace(clr_abi_namespace());

    w.write("%struct", indent{});
    if (auto info = is_deprecated(m_type))
    {
        w.write('\n');
        write_deprecation_message(w, *info);
        w.write("%", indent{});
    }
    else
    {
        w.write(' ');
    }

    w.write(R"^-^(%
%{
)^-^", cpp_abi_name(), indent{});

    for (auto const& member : members)
    {
        if (auto info = is_deprecated(member.field))
        {
            write_deprecation_message(w, *info, 1);
        }

        w.write("%% %;\n", indent{ 1 }, [&](writer& w) { member.type->write_cpp_abi_type(w); }, member.field.Name());
    }

    w.write("%};\n", indent{});

    w.pop_namespace();
    w.pop_contract_guards(contractDepth);
    w.write('\n');
}

void delegate_type::write_cpp_forward_declaration(writer& w) const
{
    if (!w.should_declare(m_mangledName))
    {
        return;
    }

    w.write(R"^-^(#ifndef __%_FWD_DEFINED__
#define __%_FWD_DEFINED__
)^-^", bind<write_mangled_name>(m_mangledName), bind<write_mangled_name>(m_mangledName));

    w.push_namespace(clr_abi_namespace());
    w.write("%interface %;\n", indent{}, m_abiName);
    w.pop_namespace();

    w.write(R"^-^(#define % %

#endif // __%_FWD_DEFINED__

)^-^",
        bind<write_mangled_name>(m_mangledName),
        bind<write_cpp_fully_qualified_type>(clr_abi_namespace(), m_abiName),
        bind<write_mangled_name>(m_mangledName));
}

void delegate_type::write_cpp_generic_param_abi_type(writer& w) const
{
    write_cpp_abi_type(w);
}

void delegate_type::write_cpp_abi_type(writer& w) const
{
    w.write("%*", bind<write_cpp_fully_qualified_type>(clr_abi_namespace(), cpp_abi_name()));
}

inline void write_function_declaration(writer& w, function_def const& func)
{
    if (auto info = is_deprecated(func.def))
    {
        write_deprecation_message(w, *info, 1);
    }

    // If this is an overload, use the unique name
    auto fnName = func.def.Name();
    if (auto overloadAttr = get_attribute(func.def, metadata_namespace, "OverloadAttribute"sv))
    {
        auto sig = overloadAttr.Value();
        auto const& fixedArgs = sig.FixedArgs();
        XLANG_ASSERT(fixedArgs.size() == 1);
        fnName = std::get<std::string_view>(std::get<ElemSig>(fixedArgs[0].value).value);
    }

    w.write("%virtual HRESULT STDMETHODCALLTYPE %(", indent{ 1 }, fnName);

    std::string_view prefix = "\n"sv;
    for (auto const& param : func.params)
    {
        auto constMod = is_const(param.signature) ? "const "sv : ""sv;
        auto refMod = param.signature.ByRef() ? "*"sv : ""sv;
        w.write("%%%%% %",
            prefix,
            indent{ 2 },
            constMod,
            [&](writer& w) { param.type->write_cpp_abi_type(w); },
            refMod,
            param.name);
        prefix = ",\n";
    }

    if (func.return_type)
    {
        if (func.return_type->signature.Type().is_szarray())
        {
            w.write("%%unsigned int* %Length", prefix, indent{ 2 }, func.return_type->name);
            prefix = ",\n";
        }

        w.write("%%%* %",
            prefix,
            indent{ 2 },
            [&](writer& w) { func.return_type->type->write_cpp_abi_type(w); },
            func.return_type->name);
    }

    if (func.params.empty() && !func.return_type)
    {
        w.write("void) = 0;\n");
    }
    else
    {
        w.write("\n%) = 0;\n", indent{ 2 });
    }
}

template <typename T>
inline void write_cpp_interface_definition(writer& w, T const& type)
{
    constexpr bool is_delegate = std::is_same_v<T, delegate_type>;
    constexpr bool is_interface = std::is_same_v<T, interface_type>;
    static_assert(is_delegate || is_interface);

    // Generics don't get generated definitions
    if (type.is_generic())
    {
        return;
    }

    auto typeName = is_delegate ? "Delegate"sv : "Interface"sv;
    w.write(R"^-^(/*
 *
 * % %
 )^-^", typeName, type.clr_full_name());

    if (auto contractInfo = contract_attributes(type.type()))
    {
        w.write(R"^-^( *
 * Introduced to % in version %
)^-^", contractInfo->type_name, bind<write_contract_version>(contractInfo->version));
    }

    if (is_interface)
    {
        if (auto exclusiveAttr = get_attribute(type.type(), metadata_namespace, "ExclusiveToAttribute"sv))
        {
            auto sig = exclusiveAttr.Value();
            auto const& fixedArgs = sig.FixedArgs();
            XLANG_ASSERT(fixedArgs.size() == 1);
            auto sysType = std::get<ElemSig::SystemType>(std::get<ElemSig>(fixedArgs[0].value).value);

            w.write(R"^-^( *
 * Interface is a part of the implementation of type %
)^-^", sysType.name);
        }

        // TODO: We should cache this info
//         auto requiredInterfaces = type.type().InterfaceImpl();
//         if (requiredInterfaces.first != requiredInterfaces.second)
//         {
//             w.write(R"^-^( *
//  * Any object which implements this interface must also implement the following interfaces:
// )^-^");
//             for (auto const& iface : requiredInterfaces)
//             {
//                 w.write(" *     ");
//                 write_type_clr(w, iface.Interface(), generic_arg_stack::empty(), format_flags::none);
//                 w.write('\n');
//             }
//         }
    }

    w.write(R"^-^( *
 */
)^-^");

    auto contractDepth = w.push_contract_guards(type.type());

    w.write(R"^-^(#if !defined(__%_INTERFACE_DEFINED__)
#define __%_INTERFACE_DEFINED__
)^-^", type.mangled_name(), type.mangled_name());

    w.push_namespace(type.clr_abi_namespace());

    auto iidStr = type_iid(type.type());
    w.write(R"^-^(%MIDL_INTERFACE("%")
)^-^", indent{}, std::string_view{ iidStr.data(), iidStr.size() - 1 });

    if (auto info = is_deprecated(type.type()))
    {
        write_deprecation_message(w, *info);
    }

    w.write(R"^-^(%% : public IUnknown
%{
%public:
)^-^", indent{}, type.cpp_abi_name(), indent{}, indent{});

    if constexpr (std::is_same_v<T, delegate_type>)
    {
        write_function_declaration(w, type.invoke);
    }
    else // interface_type
    {
        for (auto const& func : type.functions)
        {
            write_function_declaration(w, func);
        }
    }

    w.write(R"^-^(%};

%extern MIDL_CONST_ID IID& IID_% = _uuidof(%);
)^-^", indent{}, indent{}, type.cpp_abi_name(), type.cpp_abi_name());

    w.pop_namespace();

    auto const iidFmt = (w.config().ns_prefix_state == ns_prefix::optional) ? "C_IID(%)"sv : "IID_%"sv;

    w.write(R"^-^(
EXTERN_C const IID %;
#endif /* !defined(__%_INTERFACE_DEFINED__) */
)^-^",
        [&](writer& w) { w.write(iidFmt, type.mangled_name()); },
        type.mangled_name());

    w.pop_contract_guards(contractDepth);
    w.write('\n');
}

void delegate_type::write_cpp_definition(writer& w) const
{
    write_cpp_interface_definition(w, *this);
}

void interface_type::write_cpp_forward_declaration(writer& w) const
{
    if (!w.should_declare(m_mangledName))
    {
        return;
    }

    w.write(R"^-^(#ifndef __%_FWD_DEFINED__
#define __%_FWD_DEFINED__
)^-^", bind<write_mangled_name>(m_mangledName), bind<write_mangled_name>(m_mangledName));

    w.push_namespace(clr_abi_namespace());
    w.write("%interface %;\n", indent{}, m_type.TypeName());
    w.pop_namespace();

    w.write(R"^-^(#define % %

#endif // __%_FWD_DEFINED__

)^-^",
        bind<write_mangled_name>(m_mangledName),
        bind<write_cpp_fully_qualified_type>(clr_abi_namespace(), m_type.TypeName()),
        bind<write_mangled_name>(m_mangledName));
}

void interface_type::write_cpp_generic_param_abi_type(writer& w) const
{
    write_cpp_abi_type(w);
}

void interface_type::write_cpp_abi_type(writer& w) const
{
    w.write("%*", bind<write_cpp_fully_qualified_type>(clr_abi_namespace(), cpp_abi_name()));
}

void interface_type::write_cpp_definition(writer& w) const
{
    write_cpp_interface_definition(w, *this);
}

void class_type::write_cpp_forward_declaration(writer& w) const
{
    if (!default_interface)
    {
        XLANG_ASSERT(false);
        xlang::throw_invalid("Cannot forward declare class '", m_clrFullName, "' since it has no default interface");
    }

    // We need to declare both the class as well as the default interface
    w.push_namespace(clr_logical_namespace());
    w.write("%class %;\n", indent{}, cpp_logical_name());
    w.pop_namespace();
    w.write('\n');

    default_interface->write_cpp_forward_declaration(w);
}

void class_type::write_cpp_generic_param_logical_type(writer& w) const
{
    w.write("%*", bind<write_cpp_fully_qualified_type>(clr_logical_namespace(), cpp_logical_name()));
}

void class_type::write_cpp_generic_param_abi_type(writer& w) const
{
    if (!default_interface)
    {
        XLANG_ASSERT(false);
        xlang::throw_invalid("Class '", m_clrFullName, "' cannot be used as a generic parameter since it has no "
            "default interface");
    }

    // The second template argument may look a bit odd, but it is correct. The default interface must be an interface,
    // which won't be aggregated and this is the simplest way to write generics correctly
    w.write("%<%*, %>",
        bind<write_cpp_fully_qualified_type>("Windows.Foundation.Internal"sv, "AggregateType"sv),
        bind<write_cpp_fully_qualified_type>(clr_logical_namespace(), cpp_logical_name()),
        [&](writer& w) { default_interface->write_cpp_generic_param_abi_type(w); });
}

void class_type::write_cpp_abi_type(writer& w) const
{
    if (!default_interface)
    {
        XLANG_ASSERT(false);
        xlang::throw_invalid("Class '", m_clrFullName, "' cannot be used as a function argument since it has no "
            "default interface");
    }

    default_interface->write_cpp_abi_type(w);
}

void class_type::write_cpp_definition(writer& w) const
{
    w.write(R"^-^(/*
 *
 * Class %
 )^-^", m_clrFullName);

    if (auto contractInfo = contract_attributes(m_type))
    {
        w.write(R"^-^( *
 * Introduced to % in version %
)^-^", contractInfo->type_name, bind<write_contract_version>(contractInfo->version));
    }

    for_each_attribute(m_type, metadata_namespace, "ActivatableAttribute"sv, [&](bool first, CustomAttribute const& attr)
    {
        if (first)
        {
            w.write(" *\n * RuntimeClass can be activated.\n");
        }

        // There are 6 constructors for the ActivatableAttribute; we only care about the two that give us contracts
        auto sig = attr.Value();
        auto const& fixedArgs = sig.FixedArgs();
        if (fixedArgs.size() == 2)
        {
            auto const& elem0 = std::get<ElemSig>(fixedArgs[0].value);
            auto const& elem1 = std::get<ElemSig>(fixedArgs[1].value);
            if (!std::holds_alternative<std::uint32_t>(elem0.value) ||
                !std::holds_alternative<std::string_view>(elem1.value))
            {
                return;
            }

            w.write(" *   Type can be activated via RoActivateInstance starting with version % of the % API contract\n",
                bind<write_contract_version>(std::get<std::uint32_t>(elem0.value)),
                std::get<std::string_view>(elem1.value));
        }
        else if (fixedArgs.size() == 3)
        {
            auto const& elem0 = std::get<ElemSig>(fixedArgs[0].value);
            auto const& elem1 = std::get<ElemSig>(fixedArgs[1].value);
            auto const& elem2 = std::get<ElemSig>(fixedArgs[2].value);
            if (!std::holds_alternative<ElemSig::SystemType>(elem0.value) ||
                !std::holds_alternative<std::uint32_t>(elem1.value) ||
                !std::holds_alternative<std::string_view>(elem2.value))
            {
                return;
            }

            w.write(" *   Type can be activated via the % interface starting with version % of the % API contract\n",
                std::get<ElemSig::SystemType>(elem0.value).name,
                bind<write_contract_version>(std::get<std::uint32_t>(elem1.value)),
                std::get<std::string_view>(elem2.value));
        }
    });

    for_each_attribute(m_type, metadata_namespace, "StaticAttribute"sv, [&](bool first, CustomAttribute const& attr)
    {
        if (first)
        {
            w.write(" *\n * RuntimeClass contains static methods.\n");
        }

        // There are 3 constructors for the ActivatableAttribute; we only care about one
        auto sig = attr.Value();
        auto const& fixedArgs = sig.FixedArgs();
        if (fixedArgs.size() != 3)
        {
            return;
        }

        auto const& contractElem = std::get<ElemSig>(fixedArgs[2].value);
        if (!std::holds_alternative<std::string_view>(contractElem.value))
        {
            return;
        }

        w.write(" *   Static Methods exist on the % interface starting with version % of the % API contract\n",
            std::get<ElemSig::SystemType>(std::get<ElemSig>(fixedArgs[0].value).value).name,
            bind<write_contract_version>(std::get<std::uint32_t>(std::get<ElemSig>(fixedArgs[1].value).value)),
            std::get<std::string_view>(contractElem.value));
    });

    // TODO: Cache these
//     auto requiredInterfaces = type.InterfaceImpl();
//     if (requiredInterfaces.first != requiredInterfaces.second)
//     {
//         w.write(R"^-^( *
//  * Class implements the following interfaces:
// )^-^");
//         auto defaultInterface = default_interface(type);
//         for (auto const& iface : requiredInterfaces)
//         {
//             w.write(" *    ");
//             write_type_clr(w, iface.Interface(), generic_arg_stack::empty(), format_flags::none);
//             if (iface.Interface() == defaultInterface)
//             {
//                 w.write(" ** Default Interface **");
//             }
//             w.write('\n');
//         }
//     }

    if (auto attr = get_attribute(m_type, metadata_namespace, "ThreadingAttribute"sv))
    {
        // There's only one constructor for ThreadingAttribute
        auto sig = attr.Value();
        auto const& fixedArgs = sig.FixedArgs();
        XLANG_ASSERT(fixedArgs.size() == 1);

        auto const& enumValue = std::get<ElemSig::EnumValue>(std::get<ElemSig>(fixedArgs[0].value).value);

        std::string_view msg = "";
        switch (std::get<std::int32_t>(enumValue.value))
        {
        case 1: msg = "Single Threaded Apartment"sv; break;
        case 2: msg = "Multi Threaded Apartment"sv; break;
        case 3: msg = "Both Single and Multi Threaded Apartment"sv; break;
        }

        if (!msg.empty())
        {
            w.write(" *\n * Class Threading Model:  %\n", msg);
        }
    }

    if (auto attr = get_attribute(m_type, metadata_namespace, "MarshalingBehaviorAttribute"sv))
    {
        // There's only one constructor for ThreadingAttribute
        auto sig = attr.Value();
        auto const& fixedArgs = sig.FixedArgs();
        XLANG_ASSERT(fixedArgs.size() == 1);

        auto const& enumValue = std::get<ElemSig::EnumValue>(std::get<ElemSig>(fixedArgs[0].value).value);

        std::string_view msg = "";
        switch (std::get<std::int32_t>(enumValue.value))
        {
        case 1: msg = "None - Class cannot be marshaled"sv; break;
        case 2: msg = "Agile - Class is agile"sv; break;
        case 3: msg = "Standard - Class marshals using the standard marshaler"sv; break;
        }

        if (!msg.empty())
        {
            w.write(" *\n * Class Marshaling Behavior:  %\n", msg);
        }
    }

    w.write(R"^-^( *
 */
)^-^");

    auto contractDepth = w.push_contract_guards(m_type);

    w.write(R"^-^(#ifndef RUNTIMECLASS_%_%_DEFINED
#define RUNTIMECLASS_%_%_DEFINED
)^-^",
        bind_list("_", namespace_range{ clr_logical_namespace() }),
        cpp_logical_name(),
        bind_list("_", namespace_range{ clr_logical_namespace() }),
        cpp_logical_name());

    if (auto info = is_deprecated(m_type))
    {
        write_deprecation_message(w, *info);
    }

    w.write(R"^-^(extern const __declspec(selectany) _Null_terminated_ WCHAR RuntimeClass_%_%[] = L"%";
#endif
)^-^",
        bind_list("_", namespace_range{ clr_logical_namespace() }),
        cpp_logical_name(),
        clr_full_name());

    w.pop_contract_guards(contractDepth);
    w.write('\n');
}

void generic_inst::write_cpp_forward_declaration(writer& w) const
{
    if (!w.should_declare(m_mangledName))
    {
        return;
    }

    // First need to make sure that all generic parameters are declared
    for (auto param : m_genericParams)
    {
        param->write_cpp_forward_declaration(w);
    }

    w.write(R"^-^(#ifndef DEF_%_USE
#define DEF_%_USE
#if !defined(RO_NO_TEMPLATE_NAME)
)^-^", m_mangledName, m_mangledName);

    w.push_inline_namespace(clr_abi_namespace());

    w.write(R"^-^(template <>
struct __declspec(uuid(")^-^");

    sha1 signatureHash;
    static constexpr std::uint8_t namespaceGuidBytes[] =
    {
        0x11, 0xf4, 0x7a, 0xd5,
        0x7b, 0x73,
        0x42, 0xc0,
        0xab, 0xae, 0x87, 0x8b, 0x1e, 0x16, 0xad, 0xee
    };
    signatureHash.append(namespaceGuidBytes, std::size(namespaceGuidBytes));
    append_signature(signatureHash);

    auto iidHash = signatureHash.finalize();
    iidHash[6] = (iidHash[6] & 0x0F) | 0x50;
    iidHash[8] = (iidHash[8] & 0x3F) | 0x80;
    w.write_printf("%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
        iidHash[0], iidHash[1], iidHash[2], iidHash[3],
        iidHash[4], iidHash[5],
        iidHash[6], iidHash[7],
        iidHash[8], iidHash[9],
        iidHash[10], iidHash[11], iidHash[12], iidHash[13], iidHash[14], iidHash[15]);

    auto const cppName = generic_type_abi_name();
    auto write_cpp_name = [&](writer& w)
    {
        w.write(cppName);
        w.write('<');

        std::string_view prefix;
        for (auto param : m_genericParams)
        {
            w.write("%", prefix);
            param->write_cpp_generic_param_logical_type(w);
            prefix = ", "sv;
        }

        w.write('>');
    };

    w.write(R"^-^("))
% : %_impl<)^-^", write_cpp_name, cppName);

    std::string_view prefix;
    for (auto param : m_genericParams)
    {
        w.write("%", prefix);
        param->write_cpp_generic_param_abi_type(w);
        prefix = ", "sv;
    }

    w.write(R"^-^(>
{
    static const wchar_t* z_get_rc_name_impl()
    {
        return L"%";
    }
};
// Define a typedef for the parameterized interface specialization's mangled name.
// This allows code which uses the mangled name for the parameterized interface to access the
// correct parameterized interface specialization.
typedef % %_t;
)^-^", m_clrFullName, write_cpp_name, m_mangledName);

    if (w.config().ns_prefix_state == ns_prefix::optional)
    {
        w.write(R"^-^(#if defined(MIDL_NS_PREFIX)
#define % ABI::Windows::Foundation::Collections::%_t
#else
#define % Windows::Foundation::Collections::%_t
#endif // MIDL_NS_PREFIX
)^-^", m_mangledName, m_mangledName, m_mangledName, m_mangledName);
    }
    else
    {
        auto nsPrefix = (w.config().ns_prefix_state == ns_prefix::always) ? "ABI::"sv : "";
        w.write(R"^-^(#define % %@::%_t
)^-^", m_mangledName, nsPrefix, clr_abi_namespace(), m_mangledName);
    }

    w.pop_inline_namespace();

    w.write(R"^-^(
#endif // !defined(RO_NO_TEMPLATE_NAME)
#endif /* DEF_%_USE */

)^-^", m_mangledName);
}

void generic_inst::write_cpp_generic_param_logical_type(writer& w) const
{
    // For generic instantiations, logical name == abi name
    write_cpp_generic_param_abi_type(w);
}

void generic_inst::write_cpp_generic_param_abi_type(writer& w) const
{
    write_cpp_abi_type(w);
}

void generic_inst::write_cpp_abi_type(writer& w) const
{
    w.write("%*", m_mangledName);
}

element_type const& element_type::from_type(xlang::meta::reader::ElementType type)
{
    static element_type const boolean_type{ "Boolean"sv, "bool"sv, "boolean"sv, "boolean"sv, "b1"sv };
    static element_type const char_type{ "Char16"sv, "wchar_t"sv, "wchar_t"sv, "wchar__zt"sv, "c2"sv };
    static element_type const u1_type{ "UInt8"sv, "::byte"sv, "::byte"sv, "byte"sv, "u1"sv };
    static element_type const i2_type{ "Int16"sv, "short"sv, "short"sv, "short"sv, "i2"sv };
    static element_type const u2_type{ "UInt16"sv, "UINT16"sv, "UINT16"sv, "UINT16"sv, "u2"sv };
    static element_type const i4_type{ "Int32"sv, "int"sv, "int"sv, "int"sv, "i4"sv };
    static element_type const u4_type{ "UInt32"sv, "UINT32"sv, "UINT32"sv, "UINT32"sv, "u4"sv };
    static element_type const i8_type{ "Int64"sv, "__int64"sv, "__int64"sv, "__z__zint64"sv, "i8"sv };
    static element_type const u8_type{ "UInt64"sv, "UINT64"sv, "UINT64"sv, "UINT64"sv, "u8"sv };
    static element_type const r4_type{ "Single"sv, "float"sv, "float"sv, "float"sv, "f4"sv };
    static element_type const r8_type{ "Double"sv, "double"sv, "double"sv, "double"sv, "f8"sv };
    static element_type const string_type{ "String"sv, "HSTRING"sv, "HSTRING"sv, "HSTRING"sv, "string"sv };
    static element_type const object_type{ "Object"sv, "IInspectable*"sv, "IInspectable*"sv, "IInspectable"sv, "cinterface(IInspectable)"sv };

    switch (type)
    {
    case ElementType::Boolean: return boolean_type;
    case ElementType::Char: return char_type;
    case ElementType::U1: return u1_type;
    case ElementType::I2: return i2_type;
    case ElementType::U2: return u2_type;
    case ElementType::I4: return i4_type;
    case ElementType::U4: return u4_type;
    case ElementType::I8: return i8_type;
    case ElementType::U8: return u8_type;
    case ElementType::R4: return r4_type;
    case ElementType::R8: return r8_type;
    case ElementType::String: return string_type;
    case ElementType::Object: return object_type;
    }

    xlang::throw_invalid("Unrecognized ElementType: ", std::to_string(static_cast<int>(type)));
}

void element_type::write_cpp_generic_param_logical_type(writer& w) const
{
    w.write(m_logicalName);
}

void element_type::write_cpp_generic_param_abi_type(writer& w) const
{
    if (m_logicalName != m_abiName)
    {
        w.write("%<%, %>",
            bind<write_cpp_fully_qualified_type>("Windows.Foundation.Internal"sv, "AggregateType"sv),
            m_logicalName,
            m_abiName);
    }
    else
    {
        w.write(m_abiName);
    }
}

void element_type::write_cpp_abi_type(writer& w) const
{
    w.write(m_abiName);
}

system_type const& system_type::from_name(std::string_view typeName)
{
    if (typeName == "Guid"sv)
    {
        static system_type const guid_type{ "Guid"sv, "GUID"sv, "g16"sv };
        return guid_type;
    }

    XLANG_ASSERT(false);
    xlang::throw_invalid("Unknown type '", typeName, "' in System namespace");
}

void system_type::write_cpp_generic_param_logical_type(writer& w) const
{
    write_cpp_generic_param_abi_type(w);
}

void system_type::write_cpp_generic_param_abi_type(writer& w) const
{
    w.write(m_cppName);
}

void system_type::write_cpp_abi_type(writer& w) const
{
    w.write(m_cppName);
}

mapped_type const* mapped_type::from_typedef(xlang::meta::reader::TypeDef const& type)
{
    if (type.TypeNamespace() == foundation_namespace)
    {
        if (type.TypeName() == "HResult"sv)
        {
            static mapped_type const hresult_type{ type, "HRESULT"sv, "struct(Windows.Foundation.HResult;i4)"sv };
            return &hresult_type;
        }
        else if (type.TypeName() == "EventRegistrationToken"sv)
        {
            static mapped_type event_token_type{ type, "struct EventRegistrationToken"sv, "struct(Windows.Foundation.EventRegistrationToken;i8)"sv };
            return &event_token_type;
        }
        else if (type.TypeName() == "AsyncStatus"sv)
        {
            static mapped_type const async_status_type{ type, "enum AsyncStatus"sv, "enum(Windows.Foundation.AsyncStatus;i4)"sv };
            return &async_status_type;
        }
        else if (type.TypeName() == "IAsyncInfo"sv)
        {
            static mapped_type const async_info_type{ type, "IAsyncInfo"sv, "{00000036-0000-0000-C000-000000000046}"sv };
            return &async_info_type;
        }
    }

    return nullptr;
}

void mapped_type::write_cpp_generic_param_logical_type(writer& w) const
{
    write_cpp_generic_param_abi_type(w);
}

void mapped_type::write_cpp_generic_param_abi_type(writer& w) const
{
    w.write(m_cppName);
}

void mapped_type::write_cpp_abi_type(writer& w) const
{
    w.write(m_cppName);
}
