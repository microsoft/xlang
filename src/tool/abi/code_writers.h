#pragma once

#include <optional>

#include "abi_writer.h"
#include "metadata_cache.h"
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

inline void write_contract_version(writer& w, unsigned int value)
{
    auto versionHigh = static_cast<int>((value & 0xFFFF0000) >> 16);
    auto versionLow = static_cast<int>(value & 0x0000FFFF);
    w.write("%.%", versionHigh, versionLow);
}

inline void write_api_contract_definitions(writer& w, type_cache const& types)
{
    using namespace std::literals;
    using namespace xlang::meta::reader;
    using namespace xlang::text;

    w.write(R"^-^(
//  API Contract Inclusion Definitions
#if !defined(SPECIFIC_API_CONTRACT_DEFINITIONS)
)^-^"sv);

    for (auto ns : types.dependent_namespaces)
    {
        auto itr = types.cache->namespaces.find(ns);
        if (itr != types.cache->namespaces.end())
        {
            for (auto const& contract : itr->second.contracts)
            {
                w.write(R"^-^(#if !defined(%)
#define % %
#endif // defined(%)

)^-^"sv,
                    bind<write_contract_macro>(contract.name.ns, contract.name.name),
                    bind<write_contract_macro>(contract.name.ns, contract.name.name), format_hex{ contract.current_version },
                    bind<write_contract_macro>(contract.name.ns, contract.name.name));
            }
        }
    }

    w.write(R"^-^(#endif // defined(SPECIFIC_API_CONTRACT_DEFINITIONS)


)^-^"sv);
}

inline void write_includes(writer& w, type_cache const& types, std::string_view fileName)
{
    // Forced dependencies
    w.write(R"^-^(// Header files for imported files
#include "inspectable.h"
#include "AsyncInfo.h"
#include "EventToken.h"
#include "windowscontracts.h"
)^-^");

    if (fileName != foundation_namespace)
    {
        w.write(R"^-^(#include "Windows.Foundation.h"
)^-^");
    }

    bool hasCollectionsDependency = false;
    for (auto ns : types.dependent_namespaces)
    {
        if (ns == collections_namespace)
        {
            // The collections header is hand-rolled
            hasCollectionsDependency = true;
        }
        else if (ns == foundation_namespace)
        {
            // This is a forced dependency
        }
        else if (ns == system_namespace)
        {
            // The "System" namespace a lie
        }
        else if (ns == fileName)
        {
            // Don't include ourself
        }
        else
        {
            w.write(R"^-^(#include "%.h"
)^-^", ns);
        }
    }

    if (hasCollectionsDependency)
    {
        w.write(R"^-^(// Importing Collections header
#include <windows.foundation.collections.h>
)^-^");
    }

    w.write("\n");
}

inline void write_cpp_fully_qualified_type(writer& w, std::string_view typeNamespace, std::string_view typeName)
{
    if (w.config().ns_prefix_state == ns_prefix::always)
    {
        w.write("ABI::");
    }
    else if (w.config().ns_prefix_state == ns_prefix::optional)
    {
        w.write("ABI_PARAMETER(");
    }

    w.write("@::%", typeNamespace, typeName);

    if (w.config().ns_prefix_state == ns_prefix::optional)
    {
        w.write(')');
    }
}

inline void write_mangled_name(writer& w, std::string_view mangledName)
{
    using namespace std::literals;
    auto const fmt = (w.config().ns_prefix_state == ns_prefix::always) ? "__x_ABI_C%"sv : "__x_%"sv;
    w.write(fmt, mangledName);
}

inline void write_cpp_interface_forward_declarations(writer& w, type_cache const& types)
{
    w.write("/* Forward Declarations */\n");

    for (auto const& type : types.delegates)
    {
        if (!type.get().is_generic())
        {
            type.get().write_cpp_forward_declaration(w);
        }
    }

    for (auto const& type : types.interfaces)
    {
        if (!type.get().is_generic())
        {
            type.get().write_cpp_forward_declaration(w);
        }
    }
}

inline void write_cpp_generic_definitions(writer& w, type_cache const& types)
{
    w.write(R"^-^(// Parameterized interface forward declarations (C++)

// Collection interface definitions
)^-^");

    for (auto const& [name, inst] : types.generic_instantiations)
    {
        inst.get().write_cpp_forward_declaration(w);
    }
}

inline void write_cpp_dependency_forward_declarations(writer& w, type_cache const& types)
{
    for (auto const& type : types.external_dependencies)
    {
        type.get().write_cpp_forward_declaration(w);
    }

    for (auto const& type : types.internal_dependencies)
    {
        type.get().write_cpp_forward_declaration(w);
    }
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

inline void write_cpp_type_definitions(writer& w, type_cache const& types)
{
    for (auto const& enumType : types.enums)
    {
        enumType.get().write_cpp_definition(w);
    }

    for (auto const& structType : types.structs)
    {
        structType.get().write_cpp_definition(w);
    }

    for (auto const& delegateType : types.delegates)
    {
        delegateType.get().write_cpp_definition(w);
    }

    for (auto const& interfaceType : types.interfaces)
    {
        interfaceType.get().write_cpp_definition(w);
    }

    for (auto const& classType : types.classes)
    {
        classType.get().write_cpp_definition(w);
    }
}




#if 0
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

#endif
