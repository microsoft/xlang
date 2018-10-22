#pragma once

#include <cctype>
#include <string_view>
#include <utility>

#include "namespace_iterator.h"

constexpr std::string_view system_namespace = "System";

constexpr std::string_view foundation_namespace = "Windows.Foundation";

constexpr std::string_view collections_namespace = "Windows.Foundation.Collections";

constexpr std::string_view metadata_namespace = "Windows.Foundation.Metadata";
constexpr std::string_view api_contract_attribute = "ApiContractAttribute";
constexpr std::string_view contract_version_attribute = "ContractVersionAttribute";
constexpr std::string_view default_attribute = "DefaultAttribute";
constexpr std::string_view deprecated_attribute = "DeprecatedAttribute";
constexpr std::string_view exclusive_to_attribute = "ExclusiveToAttribute";
constexpr std::string_view guid_attribute = "GuidAttribute";
constexpr std::string_view marshaling_behavior_attribute = "MarshalingBehaviorAttribute";
constexpr std::string_view overload_attribute = "OverloadAttribute";
constexpr std::string_view previous_contract_version_attribute = "PreviousContractVersionAttribute";
constexpr std::string_view threading_attribute = "ThreadingAttribute";

constexpr std::string_view internal_namespace = "Windows.Foundation.Internal";

struct type_name
{
    std::string_view ns;
    std::string_view name;

    bool operator<(type_name const& other) const
    {
        if (auto cmp = ns.compare(other.ns); cmp != 0)
        {
            return cmp < 0;
        }
        else
        {
            // Same namespace
            return name < other.name;
        }
    }

    // So that this type can be used in contexts that call 'TypeName' and 'TypeNamespace'
    auto TypeNamespace() { return this->ns; }
    auto TypeName() { return this->name; }
};

struct typename_compare
{
    template <typename LhsT, typename RhsT>
    bool operator()(LhsT const& lhs, RhsT const& rhs) const noexcept
    {
        auto cmp = lhs.TypeNamespace().compare(rhs.TypeNamespace());
        if (cmp == 0)
        {
            cmp = lhs.TypeName().compare(rhs.TypeName());
        }

        return cmp < 0;
    }
};

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

    std::string output_directory;

    std::pair<bool, type_name> map_type(std::string_view typeNamespace, std::string_view typeName) const noexcept
    {
        using namespace std::literals;

        bool isMapped = false;
        if (typeNamespace == foundation_namespace)
        {
            if (typeName == "EventRegistrationToken"sv)
            {
                // Windows.Foundation.EventRegistrationToken -> EventRegistrationToken
                isMapped = true;
                typeNamespace = {};
            }
            else if (typeName == "IAsyncInfo"sv)
            {
                // Windows.Foundation.IAsyncInfo -> IAsyncInfo
                isMapped = true;
                typeNamespace = {};
            }
            else if (typeName == "AsyncStatus"sv)
            {
                // Windows.Foundation.AsyncStatus -> AsyncStatus
                isMapped = true;
                typeNamespace = {};
            }
        }

        return { isMapped, { typeNamespace, typeName } };
    }
};

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

inline xlang::meta::reader::TypeDef const& find_required(xlang::meta::reader::TypeDef const& type) noexcept
{
    return type;
}

inline bool is_fully_specialized(xlang::meta::reader::GenericTypeInstSig const& type) noexcept
{
    using namespace xlang::meta::reader;
    for (auto const& arg : type.GenericArgs())
    {
        if (std::holds_alternative<GenericTypeIndex>(arg.Type()))
        {
            return false;
        }
        else if (std::holds_alternative<GenericTypeInstSig>(arg.Type()) &&
            !is_fully_specialized(std::get<GenericTypeInstSig>(arg.Type())))
        {
            return false;
        }
    }

    return true;
}

inline xlang::meta::reader::coded_index<xlang::meta::reader::TypeDefOrRef> default_interface(
    xlang::meta::reader::TypeDef const& type)
{
    using namespace xlang::meta::reader;
    XLANG_ASSERT(get_category(type) == category::class_type);

    for (auto const& iface : type.InterfaceImpl())
    {
        if (get_attribute(iface, metadata_namespace, default_attribute))
        {
            return iface.Interface();
        }
    }

    xlang::throw_invalid("Type '", type.TypeNamespace(), ".", type.TypeName(), "' does not have a default interface");
}

namespace xlang
{
    template <typename... T> visit_overload(T...) -> visit_overload<T...>;
}

template <typename Visitor>
inline void visit(xlang::meta::reader::coded_index<xlang::meta::reader::TypeDefOrRef> const& type, Visitor&& visitor)
{
    using namespace xlang::meta::reader;
    switch (type.type())
    {
    case TypeDefOrRef::TypeDef: visitor(type.TypeDef()); break;
    case TypeDefOrRef::TypeRef: visitor(type.TypeRef()); break;
    case TypeDefOrRef::TypeSpec: visitor(type.TypeSpec().Signature().GenericTypeInst()); break;
    }
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

// NOTE: While theoretically possible for a type to have previous contract version attribute(s) but no contract version
//       attribute, it would be rather silly to have '#if true || ...' so ignore
inline std::optional<contract_version> contract_attributes(xlang::meta::reader::TypeDef const& type)
{
    using namespace xlang::meta::reader;

    auto contractAttr = get_attribute(type, metadata_namespace, contract_version_attribute);
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
        [&](ElemSig::SystemType type)
        {
            result.type_name = type.name;
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

    for (auto const& attr : type.CustomAttribute())
    {
        auto [ns, name] = attr.TypeNamespaceAndName();
        if ((ns == metadata_namespace) && (name == previous_contract_version_attribute))
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
    using namespace xlang::meta::reader;

    auto attr = get_attribute(type, metadata_namespace, deprecated_attribute);
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
