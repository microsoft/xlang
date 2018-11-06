#include "pch.h"

#include <cctype>
#include <cstring>

#include "abi_writer.h"
#include "code_writers.h"
#include "common.h"
#include "strings.h"

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
    XLANG_ASSERT(m_indentation >= m_namespaceStack.size()); // Otherwise mixing inline and non-inline
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
    XLANG_ASSERT(m_indentation == 0); // Otherwise mixing inline and non-inline

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

void writer::push_contract_guard(contract_version& vers)
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
    write_cpp_dependency_forward_declarations(w, types);
    write_cpp_type_definitions(w, types);

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
