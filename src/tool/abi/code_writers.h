#pragma once

#include "abi_writer.h"
#include "type_writers.h"

static std::pair<std::string_view, uint32_t> contract_attribute(xlang::meta::reader::TypeDef const& type)
{
    using namespace xlang::meta::reader;

    auto attr = get_attribute(type, metadata_namespace, contract_version_attribute);
    if (!attr)
    {
        return {};
    }

    auto sig = attr.Value();
    auto const& fixedArgs = sig.FixedArgs();

    // NOTE: Definitions of the API contracts themselves also have a ContractVersionAttribute, but with a single value
    //       used for the constructor (the current version of the contract). This function is not intended to be used
    //       with API contract types and will assert here
    if (fixedArgs.size() != 2)
    {
        XLANG_ASSERT(false);
        return {};
    }

    // ContractVersionAttribute(System.Type, UInt32) OR ContractVersionAttribute(System.String, UInt32)
    auto elemSig = std::get<ElemSig>(fixedArgs[0].value);
    std::string_view resultName;
    xlang::visit(elemSig.value,
        [&](ElemSig::SystemType type)
        {
            resultName = type.name;
        },
        [&](std::string_view name)
        {
            resultName = name;
        },
        [](auto&&)
        {
            XLANG_ASSERT(false);
        });

    auto contractVersion = std::get<uint32_t>(std::get<ElemSig>(fixedArgs[1].value).value);

    return { resultName, contractVersion };
}

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

inline void write_contract_guard_begin(writer& w, std::string_view contractName, unsigned int version)
{
    using namespace xlang::text;

    auto [ns, name] = decompose_type(contractName);
    w.write("#if % >= %", bind<write_contract_macro>(ns, name), format_hex{ version });
}

inline void write_contract_guard_end(writer& w, std::string_view contractName, unsigned int version)
{
    using namespace xlang::text;

    auto [ns, name] = decompose_type(contractName);
    w.write("#endif // % >= %", bind<write_contract_macro>(ns, name), format_hex{ version });
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
    if (typeNamespace == foundation_namespace)
    {
        if ((typeName == async_info) || (typeName == async_status))
        {
            return;
        }
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

    auto [contractName, contractVersion] = contract_attribute(type);
    if (!contractName.empty())
    {
        w.write(R"^-^( *
 * Introduced to % in version %
)^-^", contractName, bind<write_contract_version>(contractVersion));
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
            for (auto const& iface : requiredInterfaces)
            {
                w.write(" *     ");
                write_type_clr(w, iface.Interface(), generic_arg_stack::empty(), format_flags::none);
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

inline void write_enum_definition(writer& w, xlang::meta::reader::TypeDef const& type)
{
    using namespace xlang::text;
    auto const ns = type.TypeNamespace();
    auto const name = type.TypeName();

    write_type_definition_banner(w, type);

    auto [contractName, contractVersion] = contract_attribute(type);
    if (!contractName.empty())
    {
        w.write("%\n", bind<write_contract_guard_begin>(contractName, contractVersion));
    }

    w.push_namespace(ns);
    w.write(R"^-^(%enum % : int
%{
)^-^", indent{}, name, indent{});

    for (auto const& field : type.FieldList())
    {
        if (auto value = field.Constant())
        {
            w.write("%    %_% = %,\n", indent{}, name, field.Name(), value);
        }
    }

    w.write("%};\n", indent{});
    w.pop_namespace();

    if (!contractName.empty())
    {
        w.write("%\n", bind<write_contract_guard_end>(contractName, contractVersion));
    }
    w.write('\n');
}

inline void write_struct_definition(writer& w, xlang::meta::reader::TypeDef const& type)
{
    using namespace xlang::text;
    auto const ns = type.TypeNamespace();
    auto const name = type.TypeName();

    write_type_definition_banner(w, type);

    w.push_namespace(ns);
    w.write(R"^-^(%struct %
%{
)^-^", indent{}, name, indent{});

    for (auto const& field : type.FieldList())
    {
        w.write("%    % %;\n",
            indent{},
            bind<write_typesig_cpp>(field.Signature().Type(), generic_arg_stack::empty(), format_flags::none),
            field.Name());
    }

    w.write("%};\n", indent{});
    w.pop_namespace();
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
%%% : public %
%{
%public:
)^-^", indent{}, bind<write_type_guid>(type), indent{}, typePrefix, name, baseType, indent{}, indent{});

    for (auto const& methodDef : type.MethodList())
    {
        // Delegates have a declared constructor that we need to ignore
        if (methodDef.Name() == ".ctor")
        {
            XLANG_ASSERT(typeCategory == category::delegate_type);
            continue;
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
}

inline void write_class_name_definition(writer& w, xlang::meta::reader::TypeDef const& type)
{
    using namespace xlang::text;
    auto const ns = type.TypeNamespace();
    auto const name = type.TypeName();

    write_type_definition_banner(w, type);
    w.write(R"^-^(#ifndef RUNTIMECLASS_%_%_DEFINED
#define RUNTIMECLASS_%_%_DEFINED
extern const __declspec(selectany) _Null_terminated_ WCHAR RuntimeClass_%_%[] = L"%";
#endif
)^-^",
        bind_list("_", namespace_range{ ns }),
        name,
        bind_list("_", namespace_range{ ns }),
        name,
        bind_list("_", namespace_range{ ns }),
        name,
        bind<write_typedef_clr>(type, generic_arg_stack::empty(), format_flags::none));
}
