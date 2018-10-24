#pragma once

#include <optional>

#include "abi_writer.h"
#include "type_writers.h"

inline void write_include_guard(writer& w, std::string_view ns)
{
    if (w.config().lowercase_include_guard)
    {
        xlang::text::bind_list<writer::write_lowercase>("2E", namespace_range{ ns })(w);
    }
    else
    {
        xlang::text::bind_list("2E", namespace_range{ ns })(w);
    }
}

inline void write_contract_macro(writer& w, std::string_view contractNamespace, std::string_view contractTypeName)
{
    using namespace xlang::text;
    w.write("%_%_VERSION",
        bind_list<writer::write_uppercase>("_", namespace_range{ contractNamespace }),
        bind<writer::write_uppercase>(contractTypeName));
}

inline void write_underlying_enum_type(writer& w, xlang::meta::reader::TypeDef const& type)
{
    using namespace xlang::meta::reader;

    auto sig = type.FieldList().first.Signature();
    xlang::visit(sig.Type().Type(),
        [&](ElementType elem)
        {
            if (elem == ElementType::U4)
            {
                w.write("unsigned int");
            }
            else
            {
                XLANG_ASSERT(elem == ElementType::I4);
                w.write("int");
            }
        },
        [](auto const&)
        {
            XLANG_ASSERT(false);
        });
}

inline void write_forward_declaration(
    writer& w,
    xlang::meta::reader::TypeDef const& type,
    generic_arg_stack const& genericArgs = generic_arg_stack::empty())
{
    using namespace std::literals;
    using namespace xlang::meta::reader;
    using namespace xlang::text;

    // Generics should be defined, not declared
    XLANG_ASSERT(distance(type.GenericParam()) == 0);

    auto typeNamespace = type.TypeNamespace();
    auto typeName = type.TypeName();

    // Some types are "projected" and therefore should not get forward declared
    if (auto [mapped, name] = w.config().map_type(typeNamespace, typeName); mapped)
    {
        return;
    }

    if (auto [shouldDeclare, mangledName] = w.should_declare(type); shouldDeclare)
    {
        auto typeCategory = get_category(type);

        // Only interfaces (and therefore delegates) need the include guards/typedef
        if ((typeCategory == category::interface_type) || (typeCategory == category::delegate_type))
        {
            w.write(R"^-^(#ifndef __%_FWD_DEFINED__
#define __%_FWD_DEFINED__
)^-^", mangledName, mangledName);
        }

        w.push_namespace(typeNamespace);

        switch (typeCategory)
        {
        case category::interface_type:
            w.write("%interface %;\n", indent{}, typeName);
            break;

        case category::class_type:
            w.write("%class %;\n", indent{}, typeName);
            break;

        case category::enum_type:
        {
            auto enumStr = w.config().enum_class ? "MIDL_ENUM"sv : "enum"sv;
            w.write("%typedef % % : % %;\n", indent{}, enumStr, typeName, bind<write_underlying_enum_type>(type), typeName);
        }   break;

        case category::struct_type:
            w.write("%typedef struct % %;\n", indent{}, typeName, typeName);
            break;

        case category::delegate_type:
            w.write("%interface I%;\n", indent{}, typeName);
            break;
        }

        w.pop_namespace();

        if ((typeCategory == category::interface_type) || (typeCategory == category::delegate_type))
        {
            w.write(R"^-^(#define % %

#endif // __%_FWD_DEFINED__
)^-^", mangledName, bind<write_typedef_cpp>(type, genericArgs, format_flags::none), mangledName);
        }

        w.write("\n");
    }
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
                            xlang::throw_invalid("Unrecognized type in 'System' namespace: ", defOrRef.TypeName());
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

inline void write_contract_version(writer& w, unsigned int value)
{
    auto versionHigh = static_cast<int>((value & 0xFFFF0000) >> 16);
    auto versionLow = static_cast<int>(value & 0x0000FFFF);
    w.write("%.%", versionHigh, versionLow);
}

inline void write_type_definition_banner(writer& w, xlang::meta::reader::TypeDef const& type)
{
    using namespace std::literals;
    using namespace xlang::meta::reader;
    using namespace xlang::text;

    auto const typeCategory = get_category(type);
    auto const categoryString = [&]()
    {
    switch (typeCategory)
    {
        case category::interface_type: return "Interface"sv;
        case category::class_type: return "Class"sv;
        case category::enum_type: // NOTE: MIDL writes 'Struct' for enums, so we will too
        case category::struct_type: return "Struct"sv;
        case category::delegate_type: return "Delegate"sv;
        default: XLANG_ASSERT(false); return ""sv;
        }
    }();

    w.write(R"^-^(/*
 *
 * % %.%
)^-^", categoryString, type.TypeNamespace(), type.TypeName());

    if (auto contractInfo = contract_attributes(type))
    {
        w.write(R"^-^( *
 * Introduced to % in version %
)^-^", contractInfo->type_name, bind<write_contract_version>(contractInfo->version));
    }

    if (typeCategory == category::interface_type)
    {
        if (auto exclusiveAttr = get_attribute(type, metadata_namespace, "ExclusiveToAttribute"sv))
        {
            auto sig = exclusiveAttr.Value();
            auto const& fixedArgs = sig.FixedArgs();
            XLANG_ASSERT(fixedArgs.size() == 1);
            auto sysType = std::get<ElemSig::SystemType>(std::get<ElemSig>(fixedArgs[0].value).value);

            w.write(R"^-^( *
 * Interface is a part of the implementation of type %
)^-^", sysType.name);
        }

        auto requiredInterfaces = type.InterfaceImpl();
        if (requiredInterfaces.first != requiredInterfaces.second)
        {
            w.write(R"^-^( *
 * Any object which implements this interface must also implement the following interfaces:
)^-^");
            for (auto const& iface : requiredInterfaces)
            {
                w.write(" *     ");
                write_type_clr(w, iface.Interface(), generic_arg_stack::empty(), format_flags::none);
                w.write('\n');
            }
        }
    }
    else if (typeCategory == category::class_type)
    {
        for_each_attribute(type, metadata_namespace, "ActivatableAttribute"sv, [&](bool first, CustomAttribute const& attr)
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

        for_each_attribute(type, metadata_namespace, "StaticAttribute"sv, [&](bool first, CustomAttribute const& attr)
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

        auto requiredInterfaces = type.InterfaceImpl();
        if (requiredInterfaces.first != requiredInterfaces.second)
        {
            w.write(R"^-^( *
 * Class implements the following interfaces:
)^-^");
            auto defaultInterface = default_interface(type);
            for (auto const& iface : requiredInterfaces)
            {
                w.write(" *    ");
                write_type_clr(w, iface.Interface(), generic_arg_stack::empty(), format_flags::none);
                if (iface.Interface() == defaultInterface)
                {
                    w.write(" ** Default Interface **");
                }
                w.write('\n');
            }
        }

        if (auto attr = get_attribute(type, metadata_namespace, "ThreadingAttribute"sv))
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

        if (auto attr = get_attribute(type, metadata_namespace, "MarshalingBehaviorAttribute"sv))
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
    }

    w.write(R"^-^( *
 */
)^-^");
}

inline void write_deprecation_message(
    writer& w,
    deprecation_info const& info,
    std::size_t additionalIndentation = 0,
    std::string_view deprecationMacro = "DEPRECATED")
{
    using namespace xlang::text;

    auto [ns, name] = decompose_type(info.contract_type);
    w.write(R"^-^(#if % >= %
%%("%")
#endif // % >= %
)^-^",
        bind<write_contract_macro>(ns, name), format_hex{ info.version },
        indent{ additionalIndentation }, deprecationMacro, info.message,
        bind<write_contract_macro>(ns, name), format_hex{ info.version });
}

inline void write_enum_definition(writer& w, xlang::meta::reader::TypeDef const& type)
{
    using namespace xlang::text;
    auto const ns = type.TypeNamespace();
    auto const name = type.TypeName();

    if (auto [isMapped, mappedName] = w.config().map_type(ns, name); isMapped)
    {
        return;
    }

    write_type_definition_banner(w, type);
    auto contractDepth = w.push_contract_guards(type);

    w.push_namespace(ns);
    w.write(w.config().enum_class ? "%MIDL_ENUM" : "%enum", indent{});
    if (auto info = is_deprecated(type))
    {
        w.write("\n");
        write_deprecation_message(w, *info);
        w.write("%", indent{});
    }
    else
    {
        w.write(' ');
    }

    w.write(R"^-^(% : %
%{
)^-^", name, bind<write_underlying_enum_type>(type), indent{});

    for (auto const& field : type.FieldList())
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

    if (is_flags_enum(type))
    {
        w.write("\n%DEFINE_ENUM_FLAG_OPERATORS(%)\n", indent{}, name);
    }

    if (w.config().enum_class)
    {
        w.write('\n');
        for (auto const& field : type.FieldList())
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

inline void write_struct_definition(writer& w, xlang::meta::reader::TypeDef const& type)
{
    using namespace xlang::text;
    auto const ns = type.TypeNamespace();
    auto const name = type.TypeName();

    if (auto [isMapped, mappedName] = w.config().map_type(ns, name); isMapped)
    {
        return;
    }

    write_type_definition_banner(w, type);
    auto contractDepth = w.push_contract_guards(type);

    w.push_namespace(ns);
    w.write("%struct", indent{});
    if (auto info = is_deprecated(type))
    {
        w.write("\n");
        write_deprecation_message(w, *info);
        w.write("%", indent{});
    }
    else
    {
        w.write(' ');
    }

    w.write(R"^-^(%
%{
)^-^", name, indent{});

    for (auto const& field : type.FieldList())
    {
        if (auto info = is_deprecated(field))
        {
            write_deprecation_message(w, *info, 1);
        }

        w.write("%% %;\n",
            indent{ 1 },
            bind<write_typesig_cpp>(field.Signature().Type(), generic_arg_stack::empty(), format_flags::none),
            field.Name());
    }

    w.write("%};\n", indent{});
    w.pop_namespace();

    w.pop_contract_guards(contractDepth);
    w.write('\n');
}

inline void write_interface_function_declaration(writer& w, xlang::meta::reader::MethodDef const& methodDef)
{
    using namespace std::literals;
    using namespace xlang::meta::reader;
    using namespace xlang::text;

    // If this is an overload, use the unique name
    auto fnName = methodDef.Name();
    if (auto overloadAttr = get_attribute(methodDef, metadata_namespace, "OverloadAttribute"sv))
    {
        auto sig = overloadAttr.Value();
        auto const& fixedArgs = sig.FixedArgs();
        XLANG_ASSERT(fixedArgs.size() == 1);
        fnName = std::get<std::string_view>(std::get<ElemSig>(fixedArgs[0].value).value);
    }

    w.write("%virtual HRESULT STDMETHODCALLTYPE %(", indent{ 1 }, fnName);

    auto paramNames = methodDef.ParamList();
    auto fnSig = methodDef.Signature();
    auto const retType = fnSig.ReturnType();

    auto paramRange = paramNames;
    if (retType && (paramRange.first != paramRange.second) && (paramRange.first.Sequence() == 0))
    {
        ++paramRange.first;
    }
    XLANG_ASSERT(xlang::meta::reader::distance(paramRange) == static_cast<std::ptrdiff_t>(fnSig.Params().size()));

    std::string_view prefix = "\n";
    for (auto const& param : fnSig.Params())
    {
        std::string_view constMod = is_const(param) ? "const " : "";
        std::string_view refMod = param.ByRef() ? "*" : "";

        if (param.Type().is_szarray())
        {
            w.write("%%unsigned int% %Length", prefix, indent{ 2 }, refMod, paramRange.first.Name());
            prefix = ",\n";
        }

        w.write("%%%%% %",
            prefix,
            indent{ 2 },
            constMod,
            bind<write_typesig_cpp>(param.Type(), generic_arg_stack::empty(), format_flags::function_param),
            refMod,
            paramRange.first.Name());
        ++paramRange.first;
        prefix = ",\n";
    }

    if (retType)
    {
        auto retName = (paramNames.first != paramRange.first) ? paramNames.first.Name() : "value";
        if (retType.Type().is_szarray())
        {
            w.write("%%unsigned int* %Length", prefix, indent{ 2 }, retName);
            prefix = ",\n";
        }

        w.write("%%%* %",
            prefix,
            indent{ 2 },
            bind<write_typesig_cpp>(retType.Type(), generic_arg_stack::empty(), format_flags::function_param),
            retName);
    }

    if ((fnSig.Params().size() == 0) && !retType)
    {
        w.write("void) = 0;\n");
    }
    else
    {
        w.write("\n%) = 0;\n", indent{ 2 });
    }
}

inline void write_interface_definition(writer& w, xlang::meta::reader::TypeDef const& type)
{
    using namespace std::literals;
    using namespace xlang::meta::reader;
    using namespace xlang::text;

    auto const ns = type.TypeNamespace();
    auto const name = type.TypeName();

    if (auto [isMapped, mappedName] = w.config().map_type(ns, name); isMapped)
    {
        return;
    }

    // Generics don't get generated definitions
    if (distance(type.GenericParam()) != 0)
    {
        return;
    }

    auto const typeCategory = get_category(type);
    auto const typePrefix = type_prefix(typeCategory);
    auto const baseType = (typeCategory == category::delegate_type) ? "IUnknown"sv : "IInspectable"sv;

    write_type_definition_banner(w, type);
    auto contractDepth = w.push_contract_guards(type);

    w.write(R"^-^(#if !defined(__%_INTERFACE_DEFINED__)
#define __%_INTERFACE_DEFINED__
)^-^",
    bind<write_typedef_mangled>(type, generic_arg_stack::empty(), format_flags::none),
    bind<write_typedef_mangled>(type, generic_arg_stack::empty(), format_flags::none));

    if (typeCategory == category::interface_type)
    {
        w.write(R"^-^(extern const __declspec(selectany) _Null_terminated_ WCHAR InterfaceName_%_%[] = L"%";
)^-^",
            bind_list("_", namespace_range{ ns }),
            name,
            bind<write_typedef_clr>(type, generic_arg_stack::empty(), format_flags::none));
    }

    w.push_namespace(ns);
    w.write(R"^-^(%MIDL_INTERFACE("%")
)^-^", indent{}, bind<write_type_guid>(type));

    if (auto info = is_deprecated(type))
    {
        write_deprecation_message(w, *info);
    }

    w.write(R"^-^(%%% : public %
%{
%public:
)^-^", indent{}, typePrefix, name, baseType, indent{}, indent{});

    for (auto const& methodDef : type.MethodList())
    {
        // Delegates have a declared constructor that we need to ignore
        if (methodDef.Name() == ".ctor")
        {
            XLANG_ASSERT(typeCategory == category::delegate_type);
            continue;
        }

        if (auto info = is_deprecated(methodDef))
        {
            write_deprecation_message(w, *info, 1);
        }

        write_interface_function_declaration(w, methodDef);
    }

    w.write(R"^-^(%};

%extern MIDL_CONST_ID IID& IID_%% = _uuidof(%%);
)^-^", indent{}, indent{}, typePrefix, name, typePrefix, name);
    w.pop_namespace();

    auto writeIid = [&](writer& w)
    {
        if (w.config().ns_prefix_state == ns_prefix::optional)
        {
            w.write("C_IID(%)", bind<write_typedef_mangled>(type, generic_arg_stack::empty(), format_flags::no_prefix));
        }
        else
        {
            w.write("IID_%", bind<write_typedef_mangled>(type, generic_arg_stack::empty(), format_flags::none));
        }
    };

    w.write(R"^-^(
EXTERN_C const IID %;
#endif /* !defined(__%_INTERFACE_DEFINED__) */
)^-^",
        writeIid,
        bind<write_typedef_mangled>(type, generic_arg_stack::empty(), format_flags::none));

    w.pop_contract_guards(contractDepth);
    w.write('\n');
}

inline void write_class_name_definition(writer& w, xlang::meta::reader::TypeDef const& type)
{
    using namespace xlang::text;
    auto const ns = type.TypeNamespace();
    auto const name = type.TypeName();

    write_type_definition_banner(w, type);
    auto contractDepth = w.push_contract_guards(type);

    w.write(R"^-^(#ifndef RUNTIMECLASS_%_%_DEFINED
#define RUNTIMECLASS_%_%_DEFINED
)^-^", bind_list("_", namespace_range{ ns }), name, bind_list("_", namespace_range{ ns }), name);

    if (auto info = is_deprecated(type))
    {
        write_deprecation_message(w, *info);
    }

    w.write(R"^-^(extern const __declspec(selectany) _Null_terminated_ WCHAR RuntimeClass_%_%[] = L"%";
#endif
)^-^",
        bind_list("_", namespace_range{ ns }),
        name,
        bind<write_typedef_clr>(type, generic_arg_stack::empty(), format_flags::none));

    w.pop_contract_guards(contractDepth);
    w.write('\n');
}
