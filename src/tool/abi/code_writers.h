#pragma once

#include <optional>

#include "abi_writer.h"
#include "types.h"

inline void write_contract_version(writer& w, unsigned int value)
{
    auto versionHigh = static_cast<int>((value & 0xFFFF0000) >> 16);
    auto versionLow = static_cast<int>(value & 0x0000FFFF);
    w.write("%.%", versionHigh, versionLow);
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

inline void write_c_type_name(writer& w, typedef_base const& type)
{
    using namespace std::literals;

    std::string_view fmt;
    switch (w.config().ns_prefix_state)
    {
    case ns_prefix::always: fmt = "__x_ABI_C%"sv; break;
    case ns_prefix::never: fmt = "__x_%"sv; break;
    case ns_prefix::optional: fmt = "C_ABI_PARAMETER(%)"sv; break;
    }

    w.write(fmt, type.mangled_name());
}

inline void write_c_type_name(writer& w, generic_inst const& type)
{
    w.write(type.mangled_name());
}

inline void write_contract_macro(writer& w, std::string_view contractNamespace, std::string_view contractTypeName)
{
    using namespace xlang::text;
    w.write("%_%_VERSION",
        bind_list<writer::write_uppercase>("_", namespace_range{ contractNamespace }),
        bind<writer::write_uppercase>(contractTypeName));
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
