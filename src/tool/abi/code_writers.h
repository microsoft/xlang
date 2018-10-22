#pragma once

#include <optional>

#include "abi_writer.h"
#include "type_writers.h"

inline void write_include_guard(writer& w, std::string_view ns)
{
    xlang::text::bind_list<writer::write_lowercase>("2E", namespace_range{ ns })(w);
}

inline void write_contract_macro(writer& w, std::string_view contractNamespace, std::string_view contractTypeName)
{
    using namespace xlang::text;
    w.write("%_%_VERSION",
        bind_list<writer::write_uppercase>("_", namespace_range{ contractNamespace }),
        bind<writer::write_uppercase>(contractTypeName));
}

inline void write_forward_declaration(
    writer& w,
    xlang::meta::reader::TypeDef const& type,
    generic_arg_stack const& genericArgs = generic_arg_stack::empty())
{
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
            // TODO: More than just int?
            w.write("%typedef enum % : int %;\n", indent{}, typeName, typeName);
            break;

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
        case category::enum_type: // TODO: return "Enum"sv; ??? 'Struct' is what MIDL does...
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
        if (auto exclusiveAttr = get_attribute(type, metadata_namespace, exclusive_to_attribute))
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
        // TODO: Activation Comment
        // TODO: Static Methods Comment

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

        // TODO: Threading model Comment
        // TODO: Marshaling behavior Comment
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

    write_type_definition_banner(w, type);
    auto contractDepth = w.push_contract_guards(type);

    w.push_namespace(ns);
    w.write("%enum", indent{});
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

    w.write(R"^-^(% : int
%{
)^-^", name, indent{});

    for (auto const& field : type.FieldList())
    {
        if (auto value = field.Constant())
        {
            w.write("%%_%", indent{ 1 }, name, field.Name());
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
        }
    }

    w.write("%};\n", indent{});
    w.pop_namespace();

    w.pop_contract_guards(contractDepth);
    w.write('\n');
}

inline void write_struct_definition(writer& w, xlang::meta::reader::TypeDef const& type)
{
    using namespace xlang::text;
    auto const ns = type.TypeNamespace();
    auto const name = type.TypeName();

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

        w.write("%    % %;\n",
            indent{},
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
    using namespace xlang::meta::reader;
    using namespace xlang::text;

    // If this is an overload, use the unique name
    auto fnName = methodDef.Name();
    if (auto overloadAttr = get_attribute(methodDef, metadata_namespace, overload_attribute))
    {
        auto sig = overloadAttr.Value();
        auto const& fixedArgs = sig.FixedArgs();
        XLANG_ASSERT(fixedArgs.size() == 1);
        fnName = std::get<std::string_view>(std::get<ElemSig>(fixedArgs[0].value).value);
    }

    w.write("%    virtual HRESULT STDMETHODCALLTYPE %(", indent{}, fnName);

    auto paramNames = methodDef.ParamList();
    auto fnSig = methodDef.Signature();
    auto const retType = fnSig.ReturnType();

    auto paramRange = paramNames;
    if (retType && (paramRange.first != paramRange.second) && (paramRange.first.Sequence() == 0))
    {
        ++paramRange.first;
    }
    XLANG_ASSERT(xlang::meta::reader::distance(paramRange) == fnSig.Params().size());

    std::string_view prefix = "\n";
    for (auto const& param : fnSig.Params())
    {
        w.write("%%        % %",
            prefix,
            indent{},
            bind<write_typesig_cpp>(param.Type(), generic_arg_stack::empty(), format_flags::function_param),
            paramRange.first.Name());
        ++paramRange.first;
        prefix = ",\n";
    }

    if (retType)
    {
        auto retName = (paramNames.first != paramRange.first) ? paramNames.first.Name() : "value";
        w.write("%%        %* %",
            prefix,
            indent{},
            bind<write_typesig_cpp>(retType.Type(), generic_arg_stack::empty(), format_flags::function_param),
            retName);
    }

    if ((fnSig.Params().size() == 0) && !retType)
    {
        w.write("void) = 0;\n");
    }
    else
    {
        w.write("\n%        ) = 0;\n", indent{});
    }
}

inline void write_interface_definition(writer& w, xlang::meta::reader::TypeDef const& type)
{
    using namespace std::literals;
    using namespace xlang::meta::reader;
    using namespace xlang::text;
    auto const ns = type.TypeNamespace();
    auto const name = type.TypeName();

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

    w.write(R"^-^(
EXTERN_C const IID IID_%;
#endif /* !defined(__%_INTERFACE_DEFINED__) */
)^-^",
        bind<write_typedef_mangled>(type, generic_arg_stack::empty(), format_flags::none),
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
