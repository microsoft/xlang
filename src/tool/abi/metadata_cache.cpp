#include "pch.h"

#include "metadata_cache.h"
#include "common.h"

using namespace std::literals;
using namespace xlang::meta::reader;

static void initialize_namespace(
    cache::namespace_members const& members,
    namespace_types& target,
    std::map<std::string_view, metadata_type const&>& table)
{
    // Mapped types are only in the 'Windows.Foundation' namespace, so pre-compute
    bool isFoundationNamespace = members.types.begin()->second.TypeNamespace() == foundation_namespace;

    target.enums.reserve(members.enums.size());
    for (auto const& e : members.enums)
    {
        // 'AsyncStatus' is an enum
        if (isFoundationNamespace)
        {
            if (auto ptr = mapped_type::from_typedef(e))
            {
                auto [itr, added] = table.emplace(e.TypeName(), *ptr);
                XLANG_ASSERT(added);
                continue;
            }
        }

        target.enums.emplace_back(e);
        auto [itr, added] = table.emplace(e.TypeName(), target.enums.back());
        XLANG_ASSERT(added);
    }

    target.structs.reserve(members.structs.size());
    for (auto const& s : members.structs)
    {
        // 'EventRegistrationToken' and 'HResult' are structs
        if (isFoundationNamespace)
        {
            if (auto ptr = mapped_type::from_typedef(s))
            {
                auto [itr, added] = table.emplace(s.TypeName(), *ptr);
                XLANG_ASSERT(added);
                continue;
            }
        }

        target.structs.emplace_back(s);
        auto [itr, added] = table.emplace(s.TypeName(), target.structs.back());
        XLANG_ASSERT(added);
    }

    target.delegates.reserve(members.delegates.size());
    for (auto const& d : members.delegates)
    {
        target.delegates.emplace_back(d);
        auto [itr, added] = table.emplace(d.TypeName(), target.delegates.back());
        XLANG_ASSERT(added);
    }

    target.interfaces.reserve(members.interfaces.size());
    for (auto const& i : members.interfaces)
    {
        // 'IAsyncInfo' is an interface
        if (isFoundationNamespace)
        {
            if (auto ptr = mapped_type::from_typedef(i))
            {
                auto [itr, added] = table.emplace(i.TypeName(), *ptr);
                XLANG_ASSERT(added);
                continue;
            }
        }

        target.interfaces.emplace_back(i);
        auto [itr, added] = table.emplace(i.TypeName(), target.interfaces.back());
        XLANG_ASSERT(added);
    }

    target.classes.reserve(members.classes.size());
    for (auto const& c : members.classes)
    {
        target.classes.emplace_back(c);
        auto [itr, added] = table.emplace(c.TypeName(), target.classes.back());
        XLANG_ASSERT(added);
    }

    for (auto const& contract : members.contracts)
    {
        // Contract versions are attributes on the contract type itself
        auto attr = get_attribute(contract, metadata_namespace, "ContractVersionAttribute"sv);
        XLANG_ASSERT(attr);
        XLANG_ASSERT(attr.Value().FixedArgs().size() == 1);

        target.contracts.insert(api_contract{
                type_name{ contract.TypeNamespace(), contract.TypeName() },
                std::get<uint32_t>(std::get<ElemSig>(attr.Value().FixedArgs()[0].value).value)
            });
    }
}

metadata_cache::metadata_cache(xlang::meta::reader::cache const& c)
{
    xlang::task_group group;
    for (auto const& [ns, members] : c.namespaces())
    {
        // We don't do any synchronization of access to this type's data structures, so reserve space on the "main"
        // thread. Note that set/map iterators are not invalidated on insert/emplace
        auto [nsItr, nsAdded] = namespaces.emplace(std::piecewise_construct,
            std::forward_as_tuple(ns),
            std::forward_as_tuple());
        XLANG_ASSERT(nsAdded);

        auto [tableItr, tableAdded] = m_typeTable.emplace(std::piecewise_construct,
            std::forward_as_tuple(ns),
            std::forward_as_tuple());
        XLANG_ASSERT(tableAdded);

        group.add([&, &nsTypes = nsItr->second, &table = tableItr->second]()
        {
            initialize_namespace(members, nsTypes, table);
        });
    }
    group.get();
}

struct type_cache_init_state
{
    type_cache* target;
};

template <typename T>
static void merge_into(std::vector<T>& from, std::vector<std::reference_wrapper<T>>& to)
{
    std::vector<std::reference_wrapper<T>> result;
    result.reserve(from.size() + to.size());
    std::merge(from.begin(), from.end(), to.begin(), to.end(), std::back_inserter(result));
    to.swap(result);
}

static void process_type(enum_type& type, type_cache_init_state& state);
static void process_type(struct_type& type, type_cache_init_state& state);
static void process_type(delegate_type& type, type_cache_init_state& state);
static void process_type(interface_type& type, type_cache_init_state& state);
static void process_type(class_type& type, type_cache_init_state& state);

type_cache metadata_cache::process_namespaces(std::initializer_list<std::string_view> targetNamespaces)
{
    type_cache result{ this };

    // Merge the type definitions of all namespaces together
    for (auto ns : targetNamespaces)
    {
        auto itr = namespaces.find(ns);
        if (itr == namespaces.end())
        {
            xlang::throw_invalid("Namespace '", ns, "' not found");
        }

        merge_into(itr->second.enums, result.enums);
        merge_into(itr->second.structs, result.structs);
        merge_into(itr->second.delegates, result.delegates);
        merge_into(itr->second.interfaces, result.interfaces);
        merge_into(itr->second.classes, result.classes);
    }

    // Process type signatures and calculate dependencies
    type_cache_init_state state{ &result };
    auto process_types = [&](auto const& vector)
    {
        for (auto const& type : vector)
        {
            process_type(type, state);
        }
    };
    process_types(result.enums);
    process_types(result.structs);
    process_types(result.delegates);
    process_types(result.interfaces);
    process_types(result.classes);

    return result;
}

template <typename T>
static void process_contract_dependencies(T const& type, type_cache_init_state& state)
{
    if (auto attr = contract_attributes(type))
    {
        state.target->dependent_namespaces.emplace(decompose_type(attr->type_name).first);
        for (auto const& prevContract : attr->previous_contracts)
        {
            state.target->dependent_namespaces.emplace(decompose_type(prevContract.type_name).first);
        }
    }

    if (auto info = is_deprecated(type))
    {
        state.target->dependent_namespaces.emplace(decompose_type(info->contract_type).first);
    }
}

static void process_type(enum_type& type, type_cache_init_state& state)
{
    // There's no pre-processing that we need to do for enums. Just take note of the namespace dependencies that come
    // from contract version(s)/deprecations
    process_contract_dependencies(type.type(), state);

    for (auto const& field : type.type().FieldList())
    {
        process_contract_dependencies(field, state);
    }
}

static void process_type(struct_type& type, type_cache_init_state& state)
{
    process_contract_dependencies(type.type(), state);

    for (auto const& field : type.type().FieldList())
    {
        process_contract_dependencies(field, state);
        type.members.push_back(struct_member{ field, &state.target->find(field.Signature().Type()) });
    }
}

static function_def process_function(MethodDef const& def, type_cache_init_state& state)
{
    auto paramNames = def.ParamList();
    auto sig = def.Signature();
    XLANG_ASSERT(sig.GenericParamCount() == 0);

    std::optional<function_return_type> return_type;
    if (sig.ReturnType())
    {
        std::string_view name = "value"sv;
        if ((paramNames.first != paramNames.second) && (paramNames.first.Sequence() == 0))
        {
            name = paramNames.first.Name();
            ++paramNames.first;
        }

        return_type = function_return_type{ sig.ReturnType(), name, &state.target->find(sig.ReturnType().Type()) };
    }

    std::vector<function_param> params;
    for (auto const& param : sig.Params())
    {
        XLANG_ASSERT(paramNames.first != paramNames.second);
        params.push_back(function_param{ param, paramNames.first.Name(), &state.target->find(param.Type()) });
        ++paramNames.first;
    }

    return function_def{ def, std::move(return_type), std::move(params) };
}

static void process_type(delegate_type& type, type_cache_init_state& state)
{
    process_contract_dependencies(type.type(), state);

    // Delegates only have a single function - Invoke - that we care about
    for (auto const& method : type.type().MethodList())
    {
        if (method.Name() != ".ctor"sv)
        {
            XLANG_ASSERT(method.Name() == "Invoke"sv);
            process_contract_dependencies(method, state);
            type.invoke = process_function(method, state);
            break;
        }
    }
}

static void process_type(interface_type& type, type_cache_init_state& state)
{
    process_contract_dependencies(type.type(), state);

    for (auto const& method : type.type().MethodList())
    {
        process_contract_dependencies(method, state);
        type.functions.push_back(process_function(method, state));
    }

    // TODO: Required interfaces? At least the generic ones?
}

static void process_type(class_type& type, type_cache_init_state& state)
{
    process_contract_dependencies(type.type(), state);

    // For classes, all we care about is the interfaces and taking note of the default interface, if there is one
    for (auto const& iface : type.type().InterfaceImpl())
    {
        process_contract_dependencies(iface, state);
        if (auto attr = get_attribute(iface, metadata_namespace, "DefaultAttribute"sv))
        {
            XLANG_ASSERT(!type.default_interface);
            type.default_interface = &state.target->find(iface.Interface());
        }
        // TODO: namespace dependency? Process generic types?
    }
}

metadata_type const* type_cache::try_find(TypeSig const& sig)
{
    metadata_type const* result = nullptr;

    // std::variant<ElementType, coded_index<TypeDefOrRef>, GenericTypeIndex, GenericTypeInstSig>;
    xlang::visit(sig.Type(),
        [&](ElementType t)
        {
            result = &element_type::from_type(t);
        },
        [&](coded_index<TypeDefOrRef> const& t)
        {
            result = try_find(t);
        },
        [&](GenericTypeIndex)
        {
            XLANG_ASSERT(false); // TODO?
        },
        [&](GenericTypeInstSig const& t)
        {
            result = try_find(t);
        });

    return result;
}

metadata_type const* type_cache::try_find(coded_index<TypeDefOrRef> const& type)
{
    metadata_type const* result = nullptr;

    visit(type, xlang::visit_overload{
        [&](GenericTypeInstSig const& sig)
        {
            result = try_find(sig);
        },
        [&](auto const& defOrRef)
        {
            result = cache->try_find(defOrRef.TypeNamespace(), defOrRef.TypeName());
        }});

    return result;
}

metadata_type const* type_cache::try_find(GenericTypeInstSig const& sig)
{
    metadata_type const* genericType = try_find(sig.GenericType());
    if (!genericType)
    {
        return nullptr;
    }

    std::vector<metadata_type const*> genericParams;
    for (auto const& param : sig.GenericArgs())
    {
        auto ptr = try_find(param);
        if (!ptr)
        {
            return nullptr;
        }
        genericParams.push_back(ptr);
    }

    generic_inst inst{ genericType, std::move(genericParams) };
    auto [itr, added] = generic_instantiations.emplace(inst.clr_full_name(), std::move(inst));
    return &itr->second;
}






#if 0
void type_cache::process_dependencies(TypeDef const& type, init_state& state)
{
    // Ignore contract definitions since they don't introduce any dependencies and their contract version attribute will
    // trip us up since it's a definition, not a use and therefore specifies no type name
    if (!get_attribute(type, metadata_namespace, "ApiContractAttribute"sv))
    {
        if (auto contractInfo = contract_attributes(type))
        {
            // NOTE: Contracts don't becomes real types; all that we really care about is the namespace dependency,
            //       primarily so that we define the contract version(s) later
            dependent_namespaces.emplace(decompose_type(contractInfo->type_name).first);
            for (auto const& prevContract : contractInfo->previous_contracts)
            {
                dependent_namespaces.emplace(decompose_type(prevContract.type_name).first);
            }
        }
    }

    // TODO: Only generic required interfaces?
    for (auto const& iface : type.InterfaceImpl())
    {
        process_dependency(iface.Interface(), state);
    }

    for (auto const& method : type.MethodList())
    {
        if (method.Name() == ".ctor"sv)
        {
            continue;
        }

        auto sig = method.Signature();
        if (auto retType = sig.ReturnType())
        {
            process_dependency(retType.Type(), state);
        }

        for (const auto& param : sig.Params())
        {
            process_dependency(param.Type(), state);
        }
    }

    for (auto const& field : type.FieldList())
    {
        process_dependency(field.Signature().Type(), state);
    }
}

metadata_type const* type_cache::process_dependency(TypeSig const& type, init_state& state)
{
    metadata_type const* result = nullptr;

    xlang::visit(type.Type(),
        [&](ElementType t)
        {
            // Does not impact the dependency graph
            result = &get_metadata_type(t);
        },
        [&](coded_index<TypeDefOrRef> const& t)
        {
            result = process_dependency(t, state);
        },
        [&](GenericTypeIndex i)
        {
            if (!state.parent_generic_inst)
            {
                // We're processing the definition of a generic type and therefore have no instantiation to reference.
                // Leave the result null, which we will assert on if we find ourselves in this code path in unexpected
                // scenarios.
            }
            else
            {
                // This is referencing a generic parameter from an earlier generic argument and thus carries no new dependencies
                XLANG_ASSERT(i.index < state.parent_generic_inst->generic_params().size());
                result = state.parent_generic_inst->generic_params()[i.index];
            }
        },
        [&](GenericTypeInstSig const& t)
        {
            result = process_dependency(t, state);
        });

    return result;
}

metadata_type const* type_cache::process_dependency(coded_index<TypeDefOrRef> const& type, init_state& state)
{
    metadata_type const* result = nullptr;

    visit(type, xlang::visit_overload{
        [&](GenericTypeInstSig const& t)
        {
            result = process_dependency(t, state);
        },
        [&](auto const& defOrRef)
        {
            auto ns = defOrRef.TypeNamespace();
            auto name = defOrRef.TypeName();
            if (ns == system_namespace)
            {
                // All types in the System namespace are mapped
                if constexpr (std::is_same_v<std::decay_t<decltype(defOrRef)>, TypeRef>)
                {
                    result = &get_system_metadata_type(defOrRef);
                }
                else
                {
                    XLANG_ASSERT(false);
                }
            }
            else
            {
                auto const& def = m_cache->find(ns, name);
                result = &def;

                if (ns == name)
                {
                    internal_dependencies.emplace(def);
                }
                else
                {
                    external_dependencies.emplace(def);
                    dependent_namespaces.emplace(ns);
                }
            }
        }});

    return result;
}

metadata_type const* type_cache::process_dependency(GenericTypeInstSig const& type, init_state& state)
{
    auto ref = type.GenericType().TypeRef();
    auto const& def = m_cache->find(ref.TypeNamespace(), ref.TypeName());

    std::vector<metadata_type const*> genericParams;

    std::string clrName{ def.clr_full_name() };
    clrName.push_back('<');

    std::string mangledName{ def.mangled_name() };

    std::string_view prefix;
    for (auto const& arg : type.GenericArgs())
    {
        // It may be the case that we're processing the dependencies of a generic type, and _not_ a generic
        // instantiation, in which case we'll get back null. If that's the case, then we should get back null for _all_
        // of the generic args
        auto param = process_dependency(arg, state);
        if (param)
        {
            genericParams.push_back(param);

            clrName += prefix;
            clrName += param->clr_full_name();
            prefix = ", "sv;

            mangledName.push_back('_');
            mangledName += param->generic_param_mangled_name();
        }
    }
    clrName.push_back('>');

    if (genericParams.size() > 0)
    {
        XLANG_ASSERT(static_cast<std::size_t>(distance(type.GenericArgs())) == genericParams.size());

        generic_inst inst{ &def, std::move(genericParams), std::move(clrName), std::move(mangledName) };
        auto [itr, added] = generic_instantiations.emplace(inst.clr_full_name(), std::move(inst));
        if (added)
        {
            // First time processing this specialization
            dependent_namespaces.emplace(ref.TypeNamespace());

            auto temp = std::exchange(state.parent_generic_inst, &itr->second);
            process_dependencies(def.type(), state);
            state.parent_generic_inst = temp;
        }

        return &itr->second;
    }

    return nullptr;
}
#endif

element_type const& element_type::from_type(xlang::meta::reader::ElementType type)
{
    static element_type const boolean_type{ "Boolean"sv, "bool"sv, "boolean"sv, "boolean"sv };
    static element_type const char_type{ "Char16"sv, "wchar_t"sv, "wchar_t"sv, "wchar__zt"sv };
    static element_type const u1_type{ "UInt8"sv, "::byte"sv, "::byte"sv, "byte"sv };
    static element_type const i2_type{ "Int16"sv, "short"sv, "short"sv, "short"sv };
    static element_type const u2_type{ "UInt16"sv, "UINT16"sv, "UINT16"sv, "UINT16"sv };
    static element_type const i4_type{ "Int32"sv, "int"sv, "int"sv, "int"sv };
    static element_type const u4_type{ "UInt32"sv, "UINT32"sv, "UINT32"sv, "UINT32"sv };
    static element_type const i8_type{ "Int64"sv, "__int64"sv, "__int64"sv, "__z__zint64"sv };
    static element_type const u8_type{ "UInt64"sv, "UINT64"sv, "UINT64"sv, "UINT64"sv };
    static element_type const r4_type{ "Single"sv, "float"sv, "float"sv, "float"sv };
    static element_type const r8_type{ "Double"sv, "double"sv, "double"sv, "double"sv };
    static element_type const string_type{ "String"sv, "HSTRING"sv, "HSTRING"sv, "HSTRING"sv };
    static element_type const object_type{ "Object"sv, "IInspectable*"sv, "IInspectable*"sv, "IInspectable"sv };

    switch (type)
    {
    case ElementType::Boolean: return boolean_type;
    case ElementType::Char: return char_type;
    case ElementType::U1: return u1_type;
    case ElementType::I2: return i2_type;
    case ElementType::U2: return u2_type;
    case ElementType::I4: return i4_type;
    case ElementType::U4: return u4_type;
    case ElementType::I8: return i8_type;
    case ElementType::U8: return u8_type;
    case ElementType::R4: return r4_type;
    case ElementType::R8: return r8_type;
    case ElementType::String: return string_type;
    case ElementType::Object: return object_type;
    }

    xlang::throw_invalid("Unrecognized ElementType: ", std::to_string(static_cast<int>(type)));
}

system_type const& system_type::from_name(std::string_view typeName)
{
    if (typeName == "Guid"sv)
    {
        static system_type const guid_type{ "Guid"sv, "GUID"sv };
        return guid_type;
    }

    xlang::throw_invalid("Unknown type '", typeName, "' in System namespace");
}

mapped_type const* mapped_type::from_typedef(xlang::meta::reader::TypeDef const& type)
{
    if (type.TypeNamespace() == foundation_namespace)
    {
        if (type.TypeName() == "HResult"sv)
        {
            static mapped_type const hresult_type{ type, "HRESULT"sv };
            return &hresult_type;
        }
        else if (type.TypeName() == "EventRegistrationToken"sv)
        {
            static mapped_type event_token_type{ type, "EventRegistrationToken"sv };
            return &event_token_type;
        }
        else if (type.TypeName() == "AsyncStatus"sv)
        {
            static mapped_type const async_status_type{ type, "AsyncStatus"sv };
            return &async_status_type;
        }
        else if (type.TypeName() == "IAsyncInfo"sv)
        {
            static mapped_type const async_info_type{ type, "IAsyncInfo"sv };
            return &async_info_type;
        }
    }

    return nullptr;
}
