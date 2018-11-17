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

struct contract_version
{
    std::string_view type_name;
    std::uint32_t version;
};

struct previous_contract
{
    std::string_view type_name;
    std::uint32_t version_introduced;
    std::uint32_t version_removed;
};

struct contract_attributes
{
    contract_version current_contract;

    // NOTE: Ordered such that the "earliest" contracts come first. E.g. the first item is the contract where the type
    //       was introduced. If the list is empty, then the above contract/version is the first contract
    std::vector<previous_contract> previous_contracts;
};

template <typename T>
inline std::optional<contract_attributes> get_contracts(T const& value)
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

    contract_attributes result;

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
    xlang::call(elemSig.value,
        [&](ElemSig::SystemType t)
        {
            result.current_contract.type_name = t.name;
        },
        [&](std::string_view name)
        {
            result.current_contract.type_name = name;
        },
        [](auto&&)
        {
            XLANG_ASSERT(false);
        });

    result.current_contract.version = read_version(std::get<ElemSig>(fixedArgs[1].value).value);

    for_each_attribute(value, metadata_namespace, "PreviousContractVersionAttribute"sv,
        [&](bool /*first*/, auto const& attr)
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
        if (prevArgs.size() == 4)
        {
            // This contract "came before" a later contract. If we've already added that contract to the list, we
            // need to insert this one before it
            auto toName = std::get<std::string_view>(std::get<ElemSig>(prevArgs[3].value).value);
            auto itr = std::find_if(result.previous_contracts.begin(), result.previous_contracts.end(),
                [&](auto const& prevContract)
            {
                return prevContract.type_name == toName;
            });
            result.previous_contracts.insert(itr, prev);
        }
        else
        {
            // This is the last contract that the type was in before moving to its current contract. Always insert
            // it at the tail
            result.previous_contracts.push_back(prev);
        }
    });

    return result;
}

template <typename T>
inline std::optional<contract_version> initial_contract(T const& value)
{
    auto attr = get_contracts(value);
    if (!attr)
    {
        return std::nullopt;
    }

    if (attr->previous_contracts.empty())
    {
        return attr->current_contract;
    }

    return contract_version{ attr->previous_contracts[0].type_name, attr->previous_contracts[0].version_introduced };
}

template <typename T>
inline std::optional<std::uint32_t> version_attribute(T const& type)
{
    using namespace std::literals;
    using namespace xlang::meta::reader;

    if (auto attr = get_attribute(type, metadata_namespace, "VersionAttribute"sv))
    {
        auto sig = attr.Value();
        auto const& prevArgs = sig.FixedArgs();

        // The version is always the first argument
        return std::get<std::uint32_t>(std::get<ElemSig>(prevArgs[0].value).value);
    }

    return std::nullopt;
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
