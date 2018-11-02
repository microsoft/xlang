#include "pch.h"

#include <cctype>
#include <cstring>

#include "abi_writer.h"
#include "code_writers.h"
#include "common.h"
#include "strings.h"
#include "type_writers.h"

using namespace std::literals;
using namespace xlang;
using namespace xlang::meta::reader;
using namespace xlang::text;

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

void writer::push_inline_namespace(std::string_view ns)
{
    XLANG_ASSERT(m_namespaceStack.empty());

    std::string_view prefix;
    if (m_config.ns_prefix_state == ns_prefix::always)
    {
        write("namespace ABI {");
        prefix = " "sv;
    }
    else if (m_config.ns_prefix_state == ns_prefix::optional)
    {
        write("ABI_NAMESPACE_BEGIN");
        prefix = " "sv;
    }

    for (auto nsPart : namespace_range{ ns })
    {
        write("%namespace % {", prefix, nsPart);
        m_namespaceStack.emplace_back(nsPart);
        prefix = " "sv;
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

void writer::pop_inline_namespace()
{
    XLANG_ASSERT(!m_namespaceStack.empty());

    std::string_view prefix;
    while (!m_namespaceStack.empty())
    {
        write("%/* % */ }", prefix, m_namespaceStack.back());
        m_namespaceStack.pop_back();
        prefix = " "sv;
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
    XLANG_ASSERT(!is_generic(type));

    // Mapped types don't carry contracts with them
    if (auto [isMapped, mappedName] = m_config.map_type(type.TypeNamespace(), type.TypeName()); isMapped)
    {
        return 0;
    }

    if (auto vers = contract_attributes(type))
    {
        return push_contract_guard(*vers);
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

std::size_t writer::push_contract_guards(xlang::meta::reader::Field const& field)
{
    if (auto vers = contract_attributes(field))
    {
        return push_contract_guard(*vers);
    }

    return 0;
}

std::size_t writer::push_contract_guard(contract_version& vers)
{
    auto [ns, name] = decompose_type(vers.type_name);
    write("#if % >= %", bind<write_contract_macro>(ns, name), format_hex{ vers.version });
    for (auto const& prev : vers.previous_contracts)
    {
        auto [prevNs, prevName] = decompose_type(prev.type_name);
        write(" || \\\n    % >= % && % < %",
            bind<write_contract_macro>(prevNs, prevName), format_hex{ prev.low_version },
            bind<write_contract_macro>(prevNs, prevName), format_hex{ prev.high_version });
    }
    write('\n');

    m_contractGuardStack.emplace_back(std::move(vers));
    return 1;
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

void write_abi_header(std::string_view fileName, abi_configuration const& config, type_cache const& types)
{
    writer w{ config };

    // All headers begin with a bit of boilerplate
    w.write(strings::file_header);
    w.write(strings::include_guard_start,
        bind<write_include_guard>(fileName),
        bind<write_include_guard>(fileName),
        bind<write_include_guard>(fileName),
        bind<write_include_guard>(fileName));
    w.write(strings::deprecated_header_start);
    w.write(strings::ns_prefix_definitions,
        (config.ns_prefix_state == ns_prefix::always) ? strings::ns_prefix_always :
        (config.ns_prefix_state == ns_prefix::optional) ? strings::ns_prefix_optional : strings::ns_prefix_never);
    if (config.ns_prefix_state == ns_prefix::optional)
    {
        w.write(strings::optional_ns_prefix_definitions);
    }
    w.write(strings::constexpr_definitions);

    write_api_contract_definitions(w, types);
    write_includes(w, types, fileName);

    // C++ interface
    w.write("#if defined(__cplusplus) && !defined(CINTERFACE)\n");
    if (config.enum_class)
    {
        w.write(strings::enum_class);
    }

    write_cpp_interface_forward_declarations(w, types);
    write_cpp_generic_definitions(w, types);

    // C interface
    w.write("#else // !defined(__cplusplus)\n");
    w.write("// C interface not currently generated\n");
    w.write("#endif // defined(__cplusplus)");

    w.write(strings::constexpr_end_definitions);
    if (config.ns_prefix_state == ns_prefix::optional)
    {
        w.write(strings::optional_ns_prefix_end_definitions);
    }
    w.write(strings::deprecated_header_end);
    w.write(strings::include_guard_end, bind<write_include_guard>(fileName), bind<write_include_guard>(fileName));

    auto filename{ config.output_directory };
    filename += fileName;
    filename += ".h";
    w.flush_to_file(filename);
}






#if 0
#if 0
#if 0
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

    push_inline_namespace(typeDef.TypeNamespace());

    auto typeName = typeDef.TypeName();
    typeName = typeName.substr(0, typeName.find('`'));
    write(R"^-^(template <>
struct __declspec(uuid("%"))
% : %%_impl<)^-^",
        bind<write_generic_type_guid>(type, m_genericArgStack),
        bind<write_generictype_cpp>(type, m_genericArgStack, format_flags::ignore_namespace),
        type_prefix(typeDef), typeName);

    std::string_view prefix;
    for (auto const& sig : type.GenericArgs())
    {
        write("%%", prefix, bind<write_generic_impl_param>(sig, m_genericArgStack));
        prefix = ", "sv;
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
)^-^",
        bind<write_generictype_clr>(type, m_genericArgStack, format_flags::none),
        bind<write_generictype_cpp>(type, m_genericArgStack, format_flags::ignore_namespace),
        mangledName);

    if (m_config.ns_prefix_state == ns_prefix::optional)
    {
        write(R"^-^(#if defined(MIDL_NS_PREFIX)
#define % ABI::@::%_t
#else
#define % @::%_t
#endif // MIDL_NS_PREFIX
)^-^",
            mangledName,
            typeDef.TypeNamespace(), mangledName,
            mangledName,
            typeDef.TypeNamespace(), mangledName);
    }
    else
    {
        write(R"^-^(#define % %%_t%
)^-^",
            mangledName,
            bind<write_namespace_open>(typeDef.TypeNamespace()),
            mangledName,
            bind<write_namespace_close>());
    }

    pop_inline_namespace();

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
        write_forward_declaration(*this, type, generic_arg_stack::empty());

        if (!includes_namespace(type.TypeNamespace()))
        {
            // For classes, we also need to declare the type's default interface
            if (get_category(type) == category::class_type)
            {
                ::visit(default_interface(type), xlang::visit_overload{
                    [](GenericTypeInstSig const&)
                    {
                        // Generic type; should have been defined already
                    },
                    [&](auto const& defOrRef)
                    {
                        write_forward_declaration(*this, find_required(defOrRef), generic_arg_stack::empty());
                    }
                });
            }
        }
    }
}

void writer::write_type_declarations()
{
    for (auto const& ref : m_namespaces)
    {
        for (auto const& e : ref.members->enums)
        {
            write_forward_declaration(*this, e, generic_arg_stack::empty());
        }
    }

    for (auto const& ref : m_namespaces)
    {
        for (auto const& s : ref.members->structs)
        {
            write_forward_declaration(*this, s, generic_arg_stack::empty());
        }
    }

    for (auto const& ref : m_namespaces)
    {
        for (auto const& c : ref.members->classes)
        {
            write_forward_declaration(*this, c, generic_arg_stack::empty());
        }
    }
}

void writer::write_type_definitions()
{
    for (auto const& ref : m_namespaces)
    {
        for (auto const& e : ref.members->enums)
        {
            write_enum_definition(*this, e);
        }
    }

    for (auto const& ref : m_namespaces)
    {
        for (auto const& s : ref.members->structs)
        {
            write_struct_definition(*this, s);
        }
    }

    for (auto const& ref : m_namespaces)
    {
        for (auto const& d : ref.members->delegates)
        {
            write_interface_definition(*this, d);
        }
    }

    for (auto const& ref : m_namespaces)
    {
        for (auto const& i : ref.members->interfaces)
        {
            write_interface_definition(*this, i);
        }
    }

    for (auto const& ref : m_namespaces)
    {
        for (auto const& c : ref.members->classes)
        {
            write_class_name_definition(*this, c);
        }
    }
}

void write_abi_header(
    std::string_view fileName,
    std::initializer_list<namespace_reference> namespaces,
    cache const& c,
    abi_configuration const& config)
{
    writer w{ namespaces, config, c };

    w.write(strings::file_header);
    w.write(strings::include_guard_start,
        bind<write_include_guard>(fileName),
        bind<write_include_guard>(fileName),
        bind<write_include_guard>(fileName),
        bind<write_include_guard>(fileName));
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
    w.write("// C interface not currently generated\n");
    w.write("#endif // defined(__cplusplus)");

    w.write(strings::constexpr_end_definitions);
    if (config.ns_prefix_state == ns_prefix::optional)
    {
        w.write(strings::optional_ns_prefix_end_definitions);
    }
    w.write(strings::deprecated_header_end);
    w.write(strings::include_guard_end, bind<write_include_guard>(fileName), bind<write_include_guard>(fileName));

    auto filename{ config.output_directory };
    filename += fileName;
    filename += ".h";
    w.flush_to_file(filename);
}
#endif
