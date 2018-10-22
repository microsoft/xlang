#include "pch.h"

#include <cctype>
#include <cstring>
#include <iostream> // TODO: REMOVE

#include "abi_writer.h"
#include "code_writers.h"
#include "common.h"
#include "strings.h"
#include "type_writers.h"

using namespace std::literals;
using namespace xlang;
using namespace xlang::meta::reader;
using namespace xlang::text;

void writer::initialize_dependencies()
{
    // Calculate a list of all types/namespaces referenced by the current namespace so that we know which files to
    // include, contracts to forward declare, etc.
    for (auto const& [name, type] : m_members.types)
    {
        initialize_dependencies(type);
    }

    // NOTE: MIDLRT will declare any generic type that references a type in this namespace, even if no function in this
    //       namespace references that generic type. We won't replicate that here for simplicity since most, if not all
    //       consumers that wish to reference such a type will include the header that contains such a function
}

void writer::initialize_dependencies(TypeDef const& type)
{
    // Ignore contract definitions since they don't introduce any dependencies and their contract version attribute will
    // trip us up since it's a definition, not a use
    if (!get_attribute(type, metadata_namespace, api_contract_attribute))
    {
        if (auto contractInfo = contract_attributes(type))
        {
            m_dependentNamespaces.emplace(decompose_type(contractInfo->type_name).first);
            for (auto const& prevContract : contractInfo->previous_contracts)
            {
                m_dependentNamespaces.emplace(decompose_type(prevContract.type_name).first);
            }
        }
    }

    for (auto const& iface : type.InterfaceImpl())
    {
        add_dependency(iface.Interface());
    }

    for (auto const& method : type.MethodList())
    {
        auto sig = method.Signature();
        add_dependency(sig.ReturnType().Type());

        for (const auto& param : sig.Params())
        {
            add_dependency(param.Type());
        }
    }
}

void writer::add_dependency(TypeSig const& type)
{
    xlang::visit(type.Type(),
        [&](ElementType const&)
        {
            // Does not impact the dependency graph
        },
        [&](coded_index<TypeDefOrRef> const& t)
        {
            add_dependency(t);
        },
        [&](GenericTypeIndex const&)
        {
            // This is referencing a generic parameter from an earlier generic argument and thus carries no new dependencies
        },
        [&](GenericTypeInstSig const& t)
        {
            add_dependency(t);
        });
}

void writer::add_dependency(coded_index<TypeDefOrRef> const& type)
{
    visit(type, xlang::visit_overload{
        [&](GenericTypeInstSig const& t)
        {
            add_dependency(t);
        },
        [&](auto const& defOrRef)
        {
            if (defOrRef.TypeNamespace() != system_namespace)
            {
                m_dependencies.emplace(find_required(defOrRef));
                m_dependentNamespaces.emplace(defOrRef.TypeNamespace());
            }
        }});
}

void writer::add_dependency(GenericTypeInstSig const& type)
{
    auto def = find_required(type.GenericType().TypeRef());
    m_dependentNamespaces.emplace(def.TypeNamespace());

    // Dependencies propagate out to all generic arguments
    for (auto const& arg : type.GenericArgs())
    {
        add_dependency(arg);
    }

    // Since we have to define any template specializations for generic types (as opposed to just forward declaring
    // them), we need to treat this type the same as we treat other types in this namespace. Since the CLR does not have
    // the same concept of specialization that C++ has, we only need to do this once on a per-generic type basis.
    auto [itr, added] = m_genericReferences.emplace(std::piecewise_construct,
        std::forward_as_tuple(type_name{ def.TypeNamespace(), def.TypeName() }),
        std::forward_as_tuple());
    if (added)
    {
        // Only process each generic type once
        initialize_dependencies(def);
    }

    // Only take note of the fully specialized generic instances; we'll walk the dependency tree later
    if (is_fully_specialized(type))
    {
        itr->second.emplace_back(type);
    }
}

void writer::push_namespace(std::string_view ns)
{
    XLANG_ASSERT(m_namespaceStack.empty());

    if (m_config.ns_prefix_state == ns_prefix::always)
    {
        write("namespace ABI {\n");
        ++m_indentation;
    }
    else if (m_config.ns_prefix_state == ns_prefix::optional)
    {
        write("ABI_NAMESPACE_BEGIN\n");
    }

    for (auto nsPart : namespace_range{ ns })
    {
        write("%namespace % {\n", indent{}, nsPart);
        m_namespaceStack.emplace_back(nsPart);
        ++m_indentation;
    }
}

void writer::push_generic_namespace(std::string_view ns)
{
    XLANG_ASSERT(m_namespaceStack.empty());

    std::string_view prefix;
    if (m_config.ns_prefix_state == ns_prefix::always)
    {
        write("namespace ABI {");
        prefix = " ";
    }
    else if (m_config.ns_prefix_state == ns_prefix::optional)
    {
        write("ABI_NAMESPACE_BEGIN");
        prefix = " ";
    }

    for (auto nsPart : namespace_range{ ns })
    {
        write("%namespace % {", prefix, nsPart);
        m_namespaceStack.emplace_back(nsPart);
        prefix = " ";
    }

    write('\n');
}

void writer::pop_namespace()
{
    XLANG_ASSERT(!m_namespaceStack.empty());
    while (!m_namespaceStack.empty())
    {
        --m_indentation;
        write("%} /* % */\n", indent{}, m_namespaceStack.back());
        m_namespaceStack.pop_back();
    }

    if (m_config.ns_prefix_state == ns_prefix::always)
    {
        write("} /* ABI */\n");
        --m_indentation;
    }
    else if (m_config.ns_prefix_state == ns_prefix::optional)
    {
        write("ABI_NAMESPACE_END\n");
    }

    XLANG_ASSERT(m_indentation == 0);
}

void writer::pop_generic_namespace()
{
    XLANG_ASSERT(!m_namespaceStack.empty());

    char const* prefix = "";
    while (!m_namespaceStack.empty())
    {
        write("%/* % */ }", prefix, m_namespaceStack.back());
        m_namespaceStack.pop_back();
        prefix = " ";
    }

    if (m_config.ns_prefix_state == ns_prefix::always)
    {
        write(" /* ABI */ }\n");
    }
    else if (m_config.ns_prefix_state == ns_prefix::optional)
    {
        write(" ABI_NAMESPACE_END\n");
    }
    else
    {
        write('\n');
    }
}

std::size_t writer::push_contract_guards(TypeDef const& type)
{
    XLANG_ASSERT(distance(type.GenericParam()) == 0);
    if (auto vers = contract_attributes(type))
    {
        auto [ns, name] = decompose_type(vers->type_name);
        write("#if % >= %", bind<write_contract_macro>(ns, name), format_hex{ vers->version });
        for (auto const& prev : vers->previous_contracts)
        {
            auto [prevNs, prevName] = decompose_type(prev.type_name);
            write(" || \\\n    % >= % && % < %",
                bind<write_contract_macro>(prevNs, prevName), format_hex{ prev.low_version },
                bind<write_contract_macro>(prevNs, prevName), format_hex{ prev.high_version });
        }
        write('\n');

        m_contractGuardStack.emplace_back(std::move(*vers));
        return 1;
    }

    return 0;
}

std::size_t writer::push_contract_guards(TypeRef const& ref)
{
    if (ref.TypeNamespace() != system_namespace)
    {
        return push_contract_guards(find_required(ref));
    }

    return 0;
}

std::size_t writer::push_contract_guards(coded_index<xlang::meta::reader::TypeDefOrRef> const& type)
{
    std::size_t result = 0;
    visit(type, [&](auto const& t)
        {
            result = push_contract_guards(t);
        });

    return result;
}

std::size_t writer::push_contract_guards(TypeSig const& type)
{
    std::size_t result = 0;

    xlang::visit(type.Type(),
        [](ElementType)
        {
            // ElementTypes don't have associated contracts
        },
        [&](GenericTypeIndex const& t)
        {
            XLANG_ASSERT(m_currentGenericArgIndex > 0);
            --m_currentGenericArgIndex;
            result = push_contract_guards(m_genericArgStack[m_currentGenericArgIndex][t.index]);
            ++m_currentGenericArgIndex;
        },
        [&](auto const& t)
        {
            result = push_contract_guards(t);
        });

    return result;
}

std::size_t writer::push_contract_guards(GenericTypeInstSig const& type)
{
    std::size_t result = 0;

    // For generics, we'll follow MIDLRT's lead and only output contract guards for the generic arguments and ignore the
    // generic type itself
    for (auto const& arg : type.GenericArgs())
    {
        result += push_contract_guards(arg);
    }

    return result;
}

void writer::pop_contract_guards(std::size_t count)
{
    while (count--)
    {
        auto const& vers = m_contractGuardStack.back();
        auto [ns, name] = decompose_type(vers.type_name);
        write("#endif // % >= %", bind<write_contract_macro>(ns, name), format_hex{ vers.version });
        for (auto const& prev : vers.previous_contracts)
        {
            auto [prevNs, prevName] = decompose_type(prev.type_name);
            write(" || \\\n       // % >= % && % < %",
                bind<write_contract_macro>(prevNs, prevName), format_hex{ prev.low_version},
                bind<write_contract_macro>(prevNs, prevName), format_hex{ prev.high_version });
        }
        write('\n');
        m_contractGuardStack.pop_back();
    }
}

std::pair<bool, std::string_view> writer::should_declare(TypeDef const& type)
{
    auto mangledName = write_temp("%", bind<write_typedef_mangled>(type, m_genericArgStack, format_flags::none));
    return should_declare(std::move(mangledName));
}

std::pair<bool, std::string_view> writer::should_declare(GenericTypeInstSig const& type)
{
    auto mangledName = write_temp("%", bind<write_generictype_mangled>(type, m_genericArgStack, format_flags::none));
    return should_declare(std::move(mangledName));
}

std::pair<bool, std::string_view> writer::should_declare(std::string mangledName)
{
    if (auto [itr, inserted] = m_typeDeclarations.emplace(std::move(mangledName)); inserted)
    {
        return { true, *itr };
    }

    return { false, {} };
}

void writer::write_api_contract_definitions()
{
    write(R"^-^(
//  API Contract Inclusion Definitions
#if !defined(SPECIFIC_API_CONTRACT_DEFINITIONS)
)^-^");

    for (auto ns : m_dependentNamespaces)
    {
        auto itr = m_cache.namespaces().find(ns);
        if (itr != m_cache.namespaces().end())
        {
            for (auto const& contract : itr->second.contracts)
            {
                // Contract versions are attributes on the contract type itself
                auto attr = get_attribute(contract, metadata_namespace, contract_version_attribute);
                XLANG_ASSERT(attr);
                XLANG_ASSERT(attr.Value().FixedArgs().size() == 1);
                auto version = std::get<uint32_t>(std::get<ElemSig>(attr.Value().FixedArgs()[0].value).value);

                write(R"^-^(#if !defined(%)
#define % %
#endif // defined(%)

)^-^",
                    bind<write_contract_macro>(contract.TypeNamespace(), contract.TypeName()),
                    bind<write_contract_macro>(contract.TypeNamespace(), contract.TypeName()), format_hex{ version },
                    bind<write_contract_macro>(contract.TypeNamespace(), contract.TypeName()));
            }
        }
    }

    write(R"^-^(#endif // defined(SPECIFIC_API_CONTRACT_DEFINITIONS)


)^-^");
}

void writer::write_includes()
{
    // Forced dependencies
    write(R"^-^(// Header files for imported files
#include "inspectable.h"
#include "AsyncInfo.h"
#include "EventToken.h"
#include "windowscontracts.h"
#include "Windows.Foundation.h"
)^-^");

    bool hasCollectionsDependency = false;
    for (auto ns : m_dependentNamespaces)
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
        else if (ns == m_namespace)
        {
            // Don't include ourself
        }
        else
        {
            write(R"^-^(#include "%.h"
)^-^", ns);
        }
    }

    if (hasCollectionsDependency)
    {
        write(R"^-^(// Importing Collections header
#include <windows.foundation.collections.h>
)^-^");
    }

    write("\n");
}

void writer::write_interface_forward_declarations()
{
    // NOTE: Delegates are, for all intents and purposes, interfaces
    write("/* Forward Declarations */\n");
    for (auto const& type : m_members.delegates)
    {
        // Generics require definitions
        if (distance(type.GenericParam()) == 0)
        {
            write_forward_declaration(*this, type, m_genericArgStack);
        }
    }

    bool const isFoundationNamespace = m_namespace == foundation_namespace;
    for (auto const& type : m_members.interfaces)
    {
        if ((distance(type.GenericParam()) == 0) && !m_config.map_type(type.TypeNamespace(), type.TypeName()).first)
        {
            write_forward_declaration(*this, type, m_genericArgStack);
        }
    }
}

void writer::write_generics_definitions()
{
    write(R"^-^(// Parameterized interface forward declarations (C++)

// Collection interface definitions
)^-^");

    for (auto const& [typeName, types] : m_genericReferences)
    {
        for (auto const& type : types)
        {
            write_generic_definition(type);
        }
    }
}

void writer::write_generic_definition(GenericTypeInstSig const& type)
{
    auto [shouldDefine, mangledName] = should_declare(type);
    if (!shouldDefine)
    {
        return;
    }

    auto typeDef = find_required(type.GenericType().TypeRef());

    // Before we can write the definition for the type, we must first write any definitions/declarations for any generic
    // parameters, required interface(s), and function return/argument types
    auto handle_typedef_or_ref = [&](coded_index<TypeDefOrRef> t)
    {
        while (t)
        {
            auto current = t;
            t = {};
            ::visit(current, xlang::visit_overload{
                [&](GenericTypeInstSig const& sig)
                {
                    write_generic_definition(sig);
                },
                [&](auto const& defOrRef)
                {
                    if (defOrRef.TypeNamespace() != system_namespace)
                    {
                        auto const& def = find_required(defOrRef);
                        write_forward_declaration(*this, def, m_genericArgStack);

                        if (get_category(def) == category::class_type)
                        {
                            t = default_interface(def);
                        }
                    }
                }});
        }
    };

    auto handle_type_sig = [&](TypeSig const& t)
    {
        xlang::visit(t.Type(),
            [&](coded_index<TypeDefOrRef> const& defOrRef)
            {
                handle_typedef_or_ref(defOrRef);
            },
            [&](GenericTypeInstSig const& sig)
            {
                write_generic_definition(sig);
            },
            [&](auto const&)
            {
                // ElementType or GenericTypeIndex. The former doesn't need declaring/defining and the latter has
                // already been taken care of
            });
    };

    for (auto const& sig : type.GenericArgs())
    {
        xlang::visit(sig.Type(),
            [&](coded_index<TypeDefOrRef> const& t)
            {
                handle_typedef_or_ref(t);
            },
            [&](GenericTypeInstSig const& t)
            {
                // TODO: push?
                write_generic_definition(t);
            },
            [](auto const&)
            {
                // Either ElementType or GenericTypeIndex. The former does not need to be declared/defined and the
                // latter should have already been taken care of when processing an earlier type
            });
    }

    XLANG_ASSERT(m_currentGenericArgIndex == m_genericArgStack.size());
    auto genericArgs = type.GenericArgs();
    m_genericArgStack.emplace_back(genericArgs.first, genericArgs.second);
    ++m_currentGenericArgIndex;

    for (auto const& iface : typeDef.InterfaceImpl())
    {
        handle_typedef_or_ref(iface.Interface());
    }

    for (auto const& method : typeDef.MethodList())
    {
        auto sig = method.Signature();
        handle_type_sig(sig.ReturnType().Type());

        for (const auto& param : sig.Params())
        {
            handle_type_sig(param.Type());
        }
    }

    XLANG_ASSERT(m_currentGenericArgIndex == m_genericArgStack.size());
    m_genericArgStack.pop_back();
    --m_currentGenericArgIndex;

    auto contractDepth = push_contract_guards(type);

    write(R"^-^(
#ifndef DEF_%_USE
#define DEF_%_USE
#if !defined(RO_NO_TEMPLATE_NAME)
)^-^", mangledName, mangledName);

    push_generic_namespace(typeDef.TypeNamespace());

    auto typeName = typeDef.TypeName();
    typeName = typeName.substr(0, typeName.find('`'));
    write(R"^-^(template <>
struct __declspec(uuid("%"))
% : %%_impl<)^-^",
        bind<write_generic_type_guid>(type, m_genericArgStack),
        bind<write_generictype_cpp>(type, m_genericArgStack, format_flags::ignore_namespace),
        type_prefix(get_category(typeDef)), typeName);

    std::string_view prefix;
    for (auto const& sig : type.GenericArgs())
    {
        write("%%", prefix, bind<write_generic_impl_param>(sig, m_genericArgStack));
        prefix = ", ";
    }

    write(R"^-^(>
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
#define % %%_t%
)^-^",
        bind<write_generictype_clr>(type, m_genericArgStack, format_flags::none),
        bind<write_generictype_cpp>(type, m_genericArgStack, format_flags::ignore_namespace),
        mangledName,
        mangledName,
        bind<write_namespace_open>(typeDef.TypeNamespace()),
        mangledName,
        bind<write_namespace_close>());

    pop_generic_namespace();

    write(R"^-^(
#endif // !defined(RO_NO_TEMPLATE_NAME)
#endif /* DEF_%_USE */

)^-^", mangledName);

    pop_contract_guards(contractDepth);
    write("\n\n");
}

void writer::write_type_dependencies()
{
    for (auto const& type : m_dependencies)
    {
        if (type.TypeNamespace() != m_namespace)
        {
            // For classes, we need to declare the type's default interface instead
            if (get_category(type) == category::class_type)
            {
                ::visit(default_interface(type), xlang::visit_overload{
                    [](GenericTypeInstSig const&)
                    {
                        // Generic type; should have been defined already
                    },
                    [&](auto const& defOrRef)
                    {
                        write_forward_declaration(*this, find_required(defOrRef), m_genericArgStack);
                    }
                });
            }
            else
            {
                write_forward_declaration(*this, type, m_genericArgStack);
            }
        }
    }
}

void writer::write_type_declarations()
{
    for (auto const& e : m_members.enums)
    {
        write_forward_declaration(*this, e, m_genericArgStack);
    }

    for (auto const& c : m_members.classes)
    {
        write_forward_declaration(*this, c, m_genericArgStack);
    }
}

void writer::write_type_definitions()
{
    for (auto const& e : m_members.enums)
    {
        write_enum_definition(*this, e);
    }

    for (auto const& s : m_members.structs)
    {
        write_struct_definition(*this, s);
    }

    for (auto const& d : m_members.delegates)
    {
        write_interface_definition(*this, d);
    }

    for (auto const& i : m_members.interfaces)
    {
        write_interface_definition(*this, i);
    }

    for (auto const& c : m_members.classes)
    {
        write_class_name_definition(*this, c);
    }
}
















void write_abi_header(std::string_view ns, cache const& c, cache::namespace_members const& members, abi_configuration const& config)
{
    writer w{ ns, config, c, members };

    w.write(strings::file_header);
    w.write(strings::include_guard_start,
        bind<write_include_guard>(ns),
        bind<write_include_guard>(ns),
        bind<write_include_guard>(ns),
        bind<write_include_guard>(ns));
    w.write(strings::deprecated_header_start);
    w.write(strings::ns_prefix_definitions,
        (config.ns_prefix_state == ns_prefix::always) ? strings::ns_prefix_always :
        (config.ns_prefix_state == ns_prefix::optional) ? strings::ns_prefix_optional : strings::ns_prefix_never);
    if (config.ns_prefix_state == ns_prefix::optional)
    {
        w.write(strings::optional_ns_prefix_definitions);
    }
    w.write(strings::constexpr_definitions);

    w.write_api_contract_definitions();
    w.write_includes();

    // C++ interface
    w.write("#if defined(__cplusplus) && !defined(CINTERFACE)\n");
    if (config.enum_class)
    {
        w.write(R"^-^(#if defined(__MIDL_USE_C_ENUM)
#define MIDL_ENUM enum
#else
#define MIDL_ENUM enum class
#endif
)^-^");
    }

    w.write_interface_forward_declarations();
    w.write_generics_definitions();
    w.write_type_dependencies();
    w.write_type_declarations();
    w.write_type_definitions();

    // C interface
    w.write("#else // !defined(__cplusplus)\n");
    // TODO: C Interface
    // TODO: Need to clear declaration cache
    w.write("#endif // defined(__cplusplus)");

    w.write(strings::constexpr_end_definitions);
    if (config.ns_prefix_state == ns_prefix::optional)
    {
        w.write(strings::optional_ns_prefix_end_definitions);
    }
    w.write(strings::deprecated_header_end);
    w.write(strings::include_guard_end, bind<write_include_guard>(ns), bind<write_include_guard>(ns));

    auto filename{ config.output_directory };
    filename += ns;
    filename += ".h";
    w.flush_to_file(filename);
}
