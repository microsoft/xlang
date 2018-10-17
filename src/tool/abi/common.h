#pragma once

#include <cctype>
#include <string_view>
#include <utility>

#include "namespace_iterator.h"

constexpr std::string_view system_namespace = "System";

constexpr std::string_view foundation_namespace = "Windows.Foundation";
constexpr std::string_view async_info = "IAsyncInfo";
constexpr std::string_view async_status = "AsyncStatus";

constexpr std::string_view collections_namespace = "Windows.Foundation.Collections";

constexpr std::string_view metadata_namespace = "Windows.Foundation.Metadata";
constexpr std::string_view api_contract_attribute = "ApiContractAttribute";
constexpr std::string_view contract_version_attribute = "ContractVersionAttribute";
constexpr std::string_view default_attribute = "DefaultAttribute";
constexpr std::string_view guid_attribute = "GuidAttribute";

constexpr std::string_view internal_namespace = "Windows.Foundation.Internal";

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
};

inline std::pair<std::string_view, std::string_view> decompose_type(std::string_view typeName)
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

inline xlang::meta::reader::TypeDef const& find_required(xlang::meta::reader::TypeDef const& type)
{
    return type;
}

inline bool is_fully_specialized(xlang::meta::reader::GenericTypeInstSig const& type)
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

    ::DebugBreak();
    XLANG_ASSERT(false);
    return {};
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
