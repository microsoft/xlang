#include "pch.h"

#include "metadata_cache.h"
#include "common.h"

using namespace std::literals;
using namespace xlang::meta::reader;

metadata_cache::metadata_cache(cache const& c)
{
    xlang::task_group group;

    // First, process all TypeDefs
    for (auto const& [ns, members] : c.namespaces())
    {
        auto [itr, added] = namespaces.emplace(std::piecewise_construct,
            std::forward_as_tuple(ns),
            std::forward_as_tuple(this, ns));
        XLANG_ASSERT(added);

        group.add([&, &target = itr->second]()
        {
            initialize_namespace(target, members);
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

void metadata_cache::initialize_namespace(namespace_cache& target, cache::namespace_members const& members)
{
    XLANG_ASSERT(!target.name.empty());

    target.enums.reserve(members.enums.size());
    for (auto const& e : members.enums)
    {
        auto [itr, added] = target.types.emplace(e.TypeName(), e);
        XLANG_ASSERT(added);
        target.enums.emplace_back(itr->second);
    }

    target.structs.reserve(members.structs.size());
    for (auto const& s : members.structs)
    {
        auto [itr, added] = target.types.emplace(s.TypeName(), s);
        XLANG_ASSERT(added);
        target.structs.emplace_back(itr->second);
    }

    target.delegates.reserve(members.delegates.size());
    for (auto const& d : members.delegates)
    {
        auto [itr, added] = target.types.emplace(d.TypeName(), d);
        XLANG_ASSERT(added);
        target.delegates.emplace_back(itr->second);
    }

    target.interfaces.reserve(members.interfaces.size());
    for (auto const& i : members.interfaces)
    {
        auto [itr, added] = target.types.emplace(i.TypeName(), i);
        XLANG_ASSERT(added);
        target.interfaces.emplace_back(itr->second);
    }

    target.classes.reserve(members.classes.size());
    for (auto const& c : members.classes)
    {
        auto [itr, added] = target.types.emplace(c.TypeName(), c);
        XLANG_ASSERT(added);
        target.classes.emplace_back(itr->second);
    }
}

void namespace_cache::process_dependencies()
{
    // NOTE: Dependencies come from: contract attributes, required interfaces, fields, and function arguments/return types
    init_state state;
    for (auto const& [nsName, type] : types)
    {
        process_dependencies(type.type_def, state);
    }
}

void namespace_cache::process_dependencies(TypeDef const& type, init_state& state)
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
        auto sig = method.Signature();
        process_dependency(sig.ReturnType().Type(), state);

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

void namespace_cache::process_dependency(TypeSig const& type, init_state& state)
{
    xlang::visit(type.Type(),
        [&](ElementType const&)
        {
            // Does not impact the dependency graph
        },
        [&](coded_index<TypeDefOrRef> const& t)
        {
            process_dependency(t, state);
        },
        [&](GenericTypeIndex const&)
        {
            // This is referencing a generic parameter from an earlier generic argument and thus carries no new dependencies
        },
        [&](GenericTypeInstSig const& t)
        {
            process_dependency(t, state);
        });
}

void namespace_cache::process_dependency(coded_index<TypeDefOrRef> const& type, init_state& state)
{
    visit(type, xlang::visit_overload{
        [&](GenericTypeInstSig const& t)
        {
            process_dependency(t, state);
        },
        [&](auto const& defOrRef)
        {
            auto ns = defOrRef.TypeNamespace();
            auto name = defOrRef.TypeName();
            if (ns == system_namespace)
            {
                // All types in the System namespace are projected
            }
            else if (ns == name)
            {
                internal_dependencies.emplace(find(name));
            }
            else
            {
                external_dependencies.emplace(m_cache->find(ns, name));
                dependent_namespaces.emplace(defOrRef.TypeNamespace());
            }
        }});
}

void namespace_cache::process_dependency(GenericTypeInstSig const& type, init_state& state)
{
    auto const& ref = type.GenericType().TypeRef();
    dependent_namespaces.emplace(ref.TypeNamespace());








    // Dependencies propagate out to all generic arguments
    auto genericArgs = type.GenericArgs();
    state.generic_param_stack.emplace_back(genericArgs.first, genericArgs.second);
    for (auto const& arg : genericArgs)
    {
        process_dependency(arg, state);
    }
    state.generic_param_stack.pop_back();

    // Since we have to define any template specializations for generic types, we need to treat this type the same as we
    // treat other types in this namespace
    // TODO: ???
    auto const& def = m_cache->find(ref.TypeNamespace(), ref.TypeName());
    if (auto [itr, added] = state.generic_types.emplace(def.clr_name); added)
    {
        process_dependencies(def.type_def, state);
    }

    // We need to calculate the full CLR name for the generic instantiation in order to determine uniqueness

}
