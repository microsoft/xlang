#pragma once

#include <cctype>
#include <string_view>
#include <utility>

#include "meta_reader.h"
#include "namespace_iterator.h"

constexpr std::string_view system_namespace = "System";
constexpr std::string_view foundation_namespace = "Windows.Foundation";
constexpr std::string_view collections_namespace = "Windows.Foundation.Collections";
constexpr std::string_view metadata_namespace = "Windows.Foundation.Metadata";

enum class ns_prefix
{
    always,
    never,
    optional,
};

struct abi_configuration
{
    bool verbose = false;
    ns_prefix ns_prefix_state = ns_prefix::always;
    bool enum_class = false;
    bool lowercase_include_guard = false;

    std::string output_directory;
};

namespace xlang
{
    template <typename... T> visit_overload(T...) -> visit_overload<T...>;
}

template <typename Visitor>
inline auto visit(xlang::meta::reader::coded_index<xlang::meta::reader::TypeDefOrRef> const& type, Visitor&& visitor)
{
    using namespace xlang::meta::reader;
    switch (type.type())
    {
    case TypeDefOrRef::TypeDef: return visitor(type.TypeDef()); break;
    case TypeDefOrRef::TypeRef: return visitor(type.TypeRef()); break;
    case TypeDefOrRef::TypeSpec: return visitor(type.TypeSpec().Signature().GenericTypeInst()); break;
    }
}

inline constexpr std::pair<std::string_view, std::string_view> decompose_type(std::string_view typeName) noexcept
{
    auto pos = typeName.find('<');
    pos = typeName.rfind('.', pos);
    if (pos == std::string_view::npos)
    {
        // No namespace
        XLANG_ASSERT(false);
        return { std::string_view{}, typeName };
    }

    return { typeName.substr(0, pos), typeName.substr(pos + 1) };
}

inline bool is_generic(xlang::meta::reader::TypeDef const& type) noexcept
{
    return distance(type.GenericParam()) != 0;
}

inline xlang::meta::reader::ElementType underlying_enum_type(xlang::meta::reader::TypeDef const& type)
{
    return std::get<xlang::meta::reader::ElementType>(type.FieldList().first.Signature().Type().Type());
}

inline xlang::meta::reader::coded_index<xlang::meta::reader::TypeDefOrRef> try_get_default_interface(xlang::meta::reader::TypeDef const& type)
{
    using namespace std::literals;
    using namespace xlang::meta::reader;
    XLANG_ASSERT(get_category(type) == category::class_type);

    for (auto const& iface : type.InterfaceImpl())
    {
        if (get_attribute(iface, metadata_namespace, "DefaultAttribute"sv))
        {
            return iface.Interface();
        }
    }

    return {};
}

struct previous_contract
{
    std::string_view type_name;
    std::uint32_t low_version;  // Introduced
    std::uint32_t high_version; // Removed
};

struct contract_version
{
    std::string_view type_name;
    std::uint32_t version;

    std::vector<previous_contract> previous_contracts;
};

template <typename T>
inline std::optional<contract_version> contract_attributes(T const& value)
{
    using namespace std::literals;
    using namespace xlang::meta::reader;

    // NOTE: While theoretically possible for a type to have previous contract version attribute(s) but no contract
    //       version attribute, it would be rather silly to have '#if true || ...' so ignore
    auto contractAttr = get_attribute(value, metadata_namespace, "ContractVersionAttribute"sv);
    if (!contractAttr)
    {
        return std::nullopt;
    }

    // Although contract version constructors accept unsigned integers as arguments, the metadata occassionally has
    // signed integers for these arguments...
    auto read_version = [](auto const& variant)
    {
        if (std::holds_alternative<std::uint32_t>(variant))
        {
            return std::get<std::uint32_t>(variant);
        }
        else
        {
            return static_cast<std::uint32_t>(std::get<std::int32_t>(variant));
        }
    };

    contract_version result;

    // The ContractVersionAttribute has three constructors, two of which are used for describing contract requirements
    // and the third describing contract definitions. This function is intended only for use with the first two
    auto sig = contractAttr.Value();
    auto const& fixedArgs = sig.FixedArgs();
    if (fixedArgs.size() != 2)
    {
        XLANG_ASSERT(false);
        return {};
    }

    auto const& elemSig = std::get<ElemSig>(fixedArgs[0].value);
    xlang::visit(elemSig.value,
        [&](ElemSig::SystemType t)
        {
            result.type_name = t.name;
        },
        [&](std::string_view name)
        {
            result.type_name = name;
        },
        [](auto&&)
        {
            XLANG_ASSERT(false);
        });

    result.version = read_version(std::get<ElemSig>(fixedArgs[1].value).value);

    for (auto const& attr : value.CustomAttribute())
    {
        auto [ns, name] = attr.TypeNamespaceAndName();
        if ((ns == metadata_namespace) && (name == "PreviousContractVersionAttribute"sv))
        {
            auto prevSig = attr.Value();
            auto const& prevArgs = prevSig.FixedArgs();

            // The PreviousContractVersionAttribute has two constructors, both of which start with the same three
            // arguments - the only ones that we care about
            previous_contract prev =
            {
                std::get<std::string_view>(std::get<ElemSig>(prevArgs[0].value).value),
                read_version(std::get<ElemSig>(prevArgs[1].value).value),
                read_version(std::get<ElemSig>(prevArgs[2].value).value),
            };
            result.previous_contracts.push_back(prev);
        }
    }

    return result;
}

struct deprecation_info
{
    std::string_view contract_type;
    std::uint32_t version;

    std::string_view message;
};

template <typename T>
inline std::optional<deprecation_info> is_deprecated(T const& type)
{
    using namespace std::literals;
    using namespace xlang::meta::reader;

    auto attr = get_attribute(type, metadata_namespace, "DeprecatedAttribute"sv);
    if (!attr)
    {
        return std::nullopt;
    }

    auto sig = attr.Value();
    auto const& fixedArgs = sig.FixedArgs();

    // There are three DeprecatedAttribute constructors, two of which deal with version numbers which we don't care
    // about here. The third is relative to a contract version, which we _do_ care about
    if ((fixedArgs.size() != 4))
    {
        return std::nullopt;
    }

    auto const& contractSig = std::get<ElemSig>(fixedArgs[3].value);
    if (!std::holds_alternative<std::string_view>(contractSig.value))
    {
        return std::nullopt;
    }

    return deprecation_info
    {
        std::get<std::string_view>(contractSig.value),
        std::get<std::uint32_t>(std::get<ElemSig>(fixedArgs[2].value).value),
        std::get<std::string_view>(std::get<ElemSig>(fixedArgs[0].value).value)
    };
}

inline bool is_flags_enum(xlang::meta::reader::TypeDef const& type)
{
    using namespace std::literals;
    return static_cast<bool>(get_attribute(type, system_namespace, "FlagsAttribute"sv));
}

template <typename T, typename Func>
inline void for_each_attribute(
    T const& type,
    std::string_view namespaceFilter,
    std::string_view typeNameFilter,
    Func&& func)
{
    bool first = true;
    for (auto const& attr : type.CustomAttribute())
    {
        auto [ns, name] = attr.TypeNamespaceAndName();
        if ((ns == namespaceFilter) && (name == typeNameFilter))
        {
            func(first, attr);
            first = false;
        }
    }
}

// NOTE: 37 characters for the null terminator; the actual string is 36 characters
inline std::array<char, 37> type_iid(xlang::meta::reader::TypeDef const& type)
{
    using namespace std::literals;
    using namespace xlang::meta::reader;

    std::array<char, 37> result;

    auto attr = get_attribute(type, metadata_namespace, "GuidAttribute"sv);
    if (!attr)
    {
        xlang::throw_invalid("'Windows.Foundation.Metadata.GuidAttribute' attribute for type '", type.TypeNamespace(),
            ".", type.TypeName(), "' not found");
    }

    auto value = attr.Value();
    auto const& args = value.FixedArgs();
    // 966BE0A7-B765-451B-AAAB-C9C498ED2594
    std::snprintf(result.data(), result.size(), "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
        std::get<uint32_t>(std::get<ElemSig>(args[0].value).value),
        std::get<uint16_t>(std::get<ElemSig>(args[1].value).value),
        std::get<uint16_t>(std::get<ElemSig>(args[2].value).value),
        std::get<uint8_t>(std::get<ElemSig>(args[3].value).value),
        std::get<uint8_t>(std::get<ElemSig>(args[4].value).value),
        std::get<uint8_t>(std::get<ElemSig>(args[5].value).value),
        std::get<uint8_t>(std::get<ElemSig>(args[6].value).value),
        std::get<uint8_t>(std::get<ElemSig>(args[7].value).value),
        std::get<uint8_t>(std::get<ElemSig>(args[8].value).value),
        std::get<uint8_t>(std::get<ElemSig>(args[9].value).value),
        std::get<uint8_t>(std::get<ElemSig>(args[10].value).value));

    return result;
}
