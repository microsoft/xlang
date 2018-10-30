#include "pch.h"

#include "metadata_cache.h"
#include "common.h"

using namespace std::literals;
using namespace xlang::meta::reader;

metadata_cache::metadata_cache(xlang::meta::reader::cache const& c) :
    m_cache(&c)
{
    xlang::task_group group;

    // First, process all TypeDefs
    for (auto const& [ns, members] : c.namespaces())
    {
        auto [itr, added] = namespaces.emplace(std::piecewise_construct,
            std::forward_as_tuple(ns),
            std::forward_as_tuple(this));
        XLANG_ASSERT(added);
        itr->second.included_namespaces.emplace_back(ns);

        auto [typesItr, typesAdded] = m_types.emplace(std::piecewise_construct,
            std::forward_as_tuple(ns),
            std::forward_as_tuple());
        XLANG_ASSERT(typesAdded);

        group.add([&, &target = itr->second, &types = typesItr->second]()
        {
            initialize_namespace(target, types, members);
        });
    }
    group.get();

    // Now that all types have been converted, process dependencies and generic instances
    for (auto& [name, nsCache] : namespaces)
    {
        group.add([&]()
        {
            nsCache.process_dependencies();
        });
    }
    group.get();
}

void metadata_cache::initialize_namespace(
    type_cache& target,
    std::map<std::string_view, type_def>& typeMap,
    cache::namespace_members const& members)
{
    target.enums.reserve(members.enums.size());
    for (auto const& e : members.enums)
    {
        auto [itr, added] = typeMap.emplace(e.TypeName(), type_def{ e });
        XLANG_ASSERT(added);
        target.types.emplace(itr->second);
        target.enums.emplace_back(itr->second);
    }

    target.structs.reserve(members.structs.size());
    for (auto const& s : members.structs)
    {
        auto [itr, added] = typeMap.emplace(s.TypeName(), type_def{ s });
        XLANG_ASSERT(added);
        target.types.emplace(itr->second);
        target.structs.emplace_back(itr->second);
    }

    target.delegates.reserve(members.delegates.size());
    for (auto const& d : members.delegates)
    {
        auto [itr, added] = typeMap.emplace(d.TypeName(), type_def{ d });
        XLANG_ASSERT(added);
        target.types.emplace(itr->second);
        target.delegates.emplace_back(itr->second);
    }

    target.interfaces.reserve(members.interfaces.size());
    for (auto const& i : members.interfaces)
    {
        auto [itr, added] = typeMap.emplace(i.TypeName(), type_def{ i });
        XLANG_ASSERT(added);
        target.types.emplace(itr->second);
        target.interfaces.emplace_back(itr->second);
    }

    target.classes.reserve(members.classes.size());
    for (auto const& c : members.classes)
    {
        auto [itr, added] = typeMap.emplace(c.TypeName(), type_def{ c });
        XLANG_ASSERT(added);
        target.types.emplace(itr->second);
        target.classes.emplace_back(itr->second);
    }

    for (auto const& contract : members.contracts)
    {
        // Contract versions are attributes on the contract type itself
        auto attr = get_attribute(contract, metadata_namespace, "ContractVersionAttribute"sv);
        XLANG_ASSERT(attr);
        XLANG_ASSERT(attr.Value().FixedArgs().size() == 1);

        target.contracts.emplace(
            type_name{ contract.TypeNamespace(), contract.TypeName() },
            std::get<uint32_t>(std::get<ElemSig>(attr.Value().FixedArgs()[0].value).value));
    }
}

void type_cache::process_dependencies()
{
    // NOTE: Dependencies come from: contract attributes, required interfaces, fields, and function arguments/return types
    init_state state;
    for (auto const& def : types)
    {
        process_dependencies(def.get().type, state);
        XLANG_ASSERT(state.parent_generic_params.empty());
    }
}

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

static type_ref make_ref(type_def const& type)
{
    return type_ref{
        type.clr_name,
        type.mangled_name,
        type.generic_param_mangled_name
    };
}

static type_ref make_ref(ElementType type)
{
    switch (type)
    {
    case ElementType::Void: return type_ref{ "INVALID"sv, "INVALID"sv, "INVALID"sv };
    case ElementType::Boolean: return type_ref{ "Boolean"sv, "INVALID"sv, "boolean"sv };
    case ElementType::Char: return type_ref{ "Char16"sv, "INVALID"sv, "wchar__zt"sv };
    case ElementType::U1: return type_ref{ "UInt8"sv, "INVALID"sv, "byte"sv };
    case ElementType::I2: return type_ref{ "Int16"sv, "INVALID"sv, "short"sv };
    case ElementType::U2: return type_ref{ "UInt16"sv, "INVALID"sv, "UINT16"sv };
    case ElementType::I4: return type_ref{ "Int32"sv, "INVALID"sv, "int"sv };
    case ElementType::U4: return type_ref{ "UInt32"sv, "INVALID"sv, "UINT32"sv };
    case ElementType::I8: return type_ref{ "Int64"sv, "INVALID"sv, "__z__zint64"sv };
    case ElementType::U8: return type_ref{ "UInt64"sv, "INVALID"sv, "UINT64"sv };
    case ElementType::R4: return type_ref{ "Single"sv, "INVALID"sv, "float"sv };
    case ElementType::R8: return type_ref{ "Double"sv, "INVALID"sv, "double"sv };
    case ElementType::String: return type_ref{ "String"sv, "INVALID"sv, "HSTRING"sv };
    case ElementType::Object: return type_ref{ "Object"sv, "INVALID"sv, "IInspectable"sv };
    }

    xlang::throw_invalid("Unrecognized ElementType: ", std::to_string(static_cast<int>(type)));
}

static type_ref make_system_ref(TypeRef const& ref)
{
    XLANG_ASSERT(ref.TypeNamespace() == system_namespace);
    if (ref.TypeName() == "Guid"sv)
    {
        return type_ref{ "Guid"sv, "INVALID"sv, "GUID"sv };
    }

    xlang::throw_invalid("Unknown type '", ref.TypeName(), "' in System namespace");
}

type_ref type_cache::process_dependency(TypeSig const& type, init_state& state)
{
    type_ref result;

    xlang::visit(type.Type(),
        [&](ElementType t)
        {
            // Does not impact the dependency graph
            result = make_ref(t);
        },
        [&](coded_index<TypeDefOrRef> const& t)
        {
            result = process_dependency(t, state);
        },
        [&](GenericTypeIndex i)
        {
            if (state.parent_generic_params.empty())
            {
                // We're processing the definition of a generic type and therefore have no instantiation to reference.
                // Leave the result empty, which we will assert on if we find ourselves in this code path in unexpected
                // scenarios.
            }
            else
            {
                // This is referencing a generic parameter from an earlier generic argument and thus carries no new dependencies
                XLANG_ASSERT(i.index < state.parent_generic_params.size());
                result = state.parent_generic_params[i.index].info;
            }
        },
        [&](GenericTypeInstSig const& t)
        {
            result = process_dependency(t, state);
        });

    return result;
}

type_ref type_cache::process_dependency(coded_index<TypeDefOrRef> const& type, init_state& state)
{
    type_ref result;

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
                // All types in the System namespace are projected
                if constexpr (std::is_same_v<std::decay_t<decltype(defOrRef)>, TypeRef>)
                {
                    result = make_system_ref(defOrRef);
                }
                else
                {
                    XLANG_ASSERT(false);
                }
            }
            else
            {
                auto const& def = m_cache->find(ns, name);
                result = make_ref(def);

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

type_ref type_cache::process_dependency(GenericTypeInstSig const& type, init_state& state)
{
    auto const& ref = type.GenericType().TypeRef();
    auto const& def = m_cache->find(ref.TypeNamespace(), ref.TypeName());

    generic_instantiation inst{ type, def };
    inst.clr_name = def.clr_name;
    inst.clr_name.push_back('<');
    inst.mangled_name = def.mangled_name;

    auto genericArgs = type.GenericArgs();

    std::string_view prefix;
    for (auto const& arg : genericArgs)
    {
        inst.generic_params.emplace_back(generic_param{ arg, process_dependency(arg, state) });
        auto& argRef = inst.generic_params.back().info;

        XLANG_ASSERT(!inst.clr_name.empty());
        inst.clr_name += prefix;
        inst.clr_name += argRef.clr_name;
        prefix = ", "sv;

        XLANG_ASSERT(!inst.mangled_name.empty());
        inst.mangled_name.push_back('_');
        inst.mangled_name += argRef.generic_param_mangled_name;
    }
    inst.clr_name.push_back('>');

    auto [itr, added] = generic_instantiations.emplace(inst.clr_name, std::move(inst));
    if (added)
    {
        // First time processing this specialization
        dependent_namespaces.emplace(ref.TypeNamespace());

        state.parent_generic_params.swap(itr->second.generic_params);
        process_dependencies(def.type, state);
        state.parent_generic_params.swap(itr->second.generic_params);
    }

    return type_ref{
        itr->second.clr_name,
        itr->second.mangled_name,
        itr->second.mangled_name
    };
}

type_cache type_cache::merge(type_cache const& other) const
{
    type_cache result{ m_cache };

    result.types = types;
    result.types.insert(other.types.begin(), other.types.end());

    result.external_dependencies = external_dependencies;
    result.external_dependencies.insert(other.external_dependencies.begin(), other.external_dependencies.end());

    result.internal_dependencies = internal_dependencies;
    result.internal_dependencies.insert(other.internal_dependencies.begin(), other.internal_dependencies.end());

    result.generic_instantiations = generic_instantiations;
    result.generic_instantiations.insert(other.generic_instantiations.begin(), other.generic_instantiations.end());

    result.contracts = contracts;
    result.contracts.insert(other.contracts.begin(), other.contracts.end());

    // NOTE: We really want 'std::set_union', but since we should only be merging unique sets, merge is more efficient
    std::merge(included_namespaces.begin(), included_namespaces.end(), other.included_namespaces.begin(), other.included_namespaces.end(), std::back_inserter(result.included_namespaces));
    std::merge(enums.begin(), enums.end(), other.enums.begin(), other.enums.end(), std::back_inserter(result.enums));
    std::merge(structs.begin(), structs.end(), other.structs.begin(), other.structs.end(), std::back_inserter(result.structs));
    std::merge(delegates.begin(), delegates.end(), other.delegates.begin(), other.delegates.end(), std::back_inserter(result.delegates));
    std::merge(interfaces.begin(), interfaces.end(), other.interfaces.begin(), other.interfaces.end(), std::back_inserter(result.interfaces));
    std::merge(classes.begin(), classes.end(), other.classes.begin(), other.classes.end(), std::back_inserter(result.classes));

    return result;
}
