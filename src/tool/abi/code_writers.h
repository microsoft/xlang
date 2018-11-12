#pragma once

#include <optional>

#include "abi_writer.h"
#include "types.h"

inline std::string_view mangled_name_macro_format(writer& w)
{
    using namespace std::literals;
    switch (w.config().ns_prefix_state)
    {
    case ns_prefix::always:
        return "__x_ABI_C%"sv;

    default:
        return "__x_%"sv;
    }
}

inline std::string_view cpp_typename_format(writer& w)
{
    using namespace std::literals;
    switch (w.config().ns_prefix_state)
    {
    case ns_prefix::always:
        return "ABI::%"sv;

    case ns_prefix::optional:
        return "ABI_PARAMETER(%)"sv;

    default:
        return "%"sv;
    }
}

inline std::string_view c_typename_format(writer& w)
{
    using namespace std::literals;
    switch (w.config().ns_prefix_state)
    {
    case ns_prefix::always:
        return "__x_ABI_C%"sv;

    case ns_prefix::optional:
        return "C_ABI_PARAMETER(%)"sv;

    default:
        return "__x_%"sv;
    }
}

inline void write_cpp_fully_qualified_type(writer& w, std::string_view typeNamespace, std::string_view typeName)
{
    w.write(cpp_typename_format(w), [&](writer& w) { w.write("@::%", typeNamespace, typeName); });
}

inline auto bind_cpp_fully_qualified_type(std::string_view typeNamespace, std::string_view typeName)
{
    return xlang::text::bind<write_cpp_fully_qualified_type>(typeNamespace, typeName);
}

inline void write_mangled_name_macro(writer& w, typedef_base const& type)
{
    w.write(mangled_name_macro_format(w), type.mangled_name());
}

inline void write_mangled_name_macro(writer& w, generic_inst const& type)
{
    w.write(type.mangled_name());
}

template <typename T>
inline auto bind_mangled_name_macro(T const& type)
{
    return [&](writer& w)
    {
        write_mangled_name_macro(w, type);
    };
}

template <typename T>
inline void write_iid_name(writer& w, T const& type)
{
    using namespace xlang::text;
    if (w.config().ns_prefix_state == ns_prefix::optional)
    {
        w.write("C_IID(%)", type.mangled_name());
    }
    else
    {
        w.write("IID_%", bind_mangled_name_macro(type));
    }
}

template <typename T>
inline auto bind_iid_name(T const& type)
{
    return [&](writer& w)
    {
        write_iid_name(w, type);
    };
}

template <typename T>
inline void write_c_type_name(writer& w, T const& type)
{
    write_c_type_name(w, type, "");
}

template <typename Suffix>
inline void write_c_type_name(writer& w, typedef_base const& type, Suffix&& suffix)
{
    w.write(c_typename_format(w), [&](writer& w) { w.write("%%", type.mangled_name(), suffix); });
}

template <typename Suffix>
inline void write_c_type_name(writer& w, generic_inst const& type, Suffix&& suffix)
{
    w.write("%%", type.mangled_name(), suffix);
}

template <typename T>
auto bind_c_type_name(T const& type)
{
    return [&](writer& w)
    {
        write_c_type_name(w, type);
    };
}

template <typename T, typename Suffix>
auto bind_c_type_name(T const& type, Suffix&& suffix)
{
    return [&](writer& w)
    {
        write_c_type_name(w, type, suffix);
    };
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
