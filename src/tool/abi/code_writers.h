#pragma once

#include <optional>

#include "abi_writer.h"
#include "metadata_cache.h"

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
                auto contractNamespace = contract.type.TypeNamespace();
                auto contractName = contract.type.TypeName();
                w.write(R"^-^(#if !defined(%)
#define % %
#endif // defined(%)

)^-^"sv,
                    bind<write_contract_macro>(contractNamespace, contractName),
                    bind<write_contract_macro>(contractNamespace, contractName), format_hex{ contract.current_version },
                    bind<write_contract_macro>(contractNamespace, contractName));
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

inline void write_c_interface_forward_declarations(writer& w, type_cache const& types)
{
    w.write("/* Forward Declarations */\n");

    for (auto const& type : types.delegates)
    {
        if (!type.get().is_generic())
        {
            type.get().write_c_forward_declaration(w);
        }
    }

    for (auto const& type : types.interfaces)
    {
        if (!type.get().is_generic())
        {
            type.get().write_c_forward_declaration(w);
        }
    }
}

inline void write_c_generic_definitions(writer& w, type_cache const& types)
{
    w.write(R"^-^(// Parameterized interface forward declarations (C)

// Collection interface definitions

)^-^");

    for (auto const& [name, inst] : types.generic_instantiations)
    {
        inst.get().write_c_forward_declaration(w);
    }
}
