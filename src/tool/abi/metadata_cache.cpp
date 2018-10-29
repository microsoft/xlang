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
        XLANG_ASSERT(state.generic_param_stack.empty());
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

static type_ref make_ref(type_cache const& type)
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
    case ElementType::Void:
        return type_ref{ "INVALID"sv, "void"sv, "INVALID"sv, "INVALID"sv };

    case ElementType::Boolean:
        // TODO: 'bool' vs 'boolean'
        return type_ref{ "Boolean"sv, "bool"sv, "INVALID"sv, "boolean"sv };

    case ElementType::Char:
        return type_ref{ "Char16"sv, "wchar_t"sv, "INVALID"sv, "wchar__zt"sv };

    case ElementType::U1:
        return type_ref{ "UInt8"sv, "::byte"sv, "INVALID"sv, "byte"sv };

    case ElementType::I2:
        return type_ref{ "Int16"sv, "short"sv, "INVALID"sv, "short"sv };

    case ElementType::U2:
        return type_ref{ "UInt16"sv, "UINT16"sv, "INVALID"sv, "UINT16"sv };

    case ElementType::I4:
        return type_ref{ "Int32"sv, "int"sv, "INVALID"sv, "int"sv };

    case ElementType::U4:
        return type_ref{ "UInt32"sv, "UINT32"sv, "INVALID"sv, "UINT32"sv };

    case ElementType::I8:
        return type_ref{ "Int64"sv, "__int64"sv, "INVALID"sv, "__z__zint64"sv };

    case ElementType::U8:
        return type_ref{ "UInt64"sv, "UINT64"sv, "INVALID"sv, "UINT64"sv };

    case ElementType::R4:
        return type_ref{ "Single"sv, "float"sv, "INVALID"sv, "float"sv };

    case ElementType::R8:
        return type_ref{ "Double"sv, "double"sv, "INVALID"sv, "double"sv };

    case ElementType::String:
        return type_ref{ "String"sv, "HSTRING"sv, "INVALID"sv, "HSTRING"sv };

    case ElementType::Object:
        return type_ref{ "Object"sv, "IInspectable*"sv, "INVALID"sv, "IInspectable"sv };
    }

    xlang::throw_invalid("Unrecognized ElementType: ", std::to_string(static_cast<int>(type)));
}

static type_ref make_system_ref(TypeRef const& ref)
{
    XLANG_ASSERT(ref.TypeNamespace() == system_namespace);
    if (ref.TypeName() == "Guid"sv)
    {
        return type_ref{ "Guid"sv, "GUID"sv, "INVALID"sv, "GUID"sv };
    }

    xlang::throw_invalid("Unknown type '", ref.TypeName(), "' in System namespace");
}

type_ref namespace_cache::process_dependency(TypeSig const& type, init_state& state)
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
        [&](GenericTypeIndex const&)
        {
            // This is referencing a generic parameter from an earlier generic argument and thus carries no new dependencies
            // TODO: result
        },
        [&](GenericTypeInstSig const& t)
        {
            result = process_dependency(t, state);
        });

    return result;
}

type_ref namespace_cache::process_dependency(coded_index<TypeDefOrRef> const& type, init_state& state)
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

type_ref namespace_cache::process_dependency(GenericTypeInstSig const& type, init_state& state)
{
    auto const& ref = type.GenericType().TypeRef();
    auto const& def = m_cache->find(ref.TypeNamespace(), ref.TypeName());

    generic_instantiation inst{ type };
    inst.clr_name = def.clr_name;
    inst.clr_name.push_back('<');
    inst.mangled_name = def.mangled_name;

    std::string_view prefix;
    auto genericArgs = type.GenericArgs();
    for (auto const& arg : genericArgs)
    {
        XLANG_ASSERT(!std::holds_alternative<GenericTypeInstSig>(arg.Type()));
        auto ref = process_dependency(arg, state);

        inst.clr_name += prefix;
        inst.clr_name += ref.clr_name;
        prefix = ", "sv;

        inst.mangled_name.push_back('_');
        inst.mangled_name += ref.generic_param_mangled_name;
    }
    inst.clr_name.push_back('>');

    auto [itr, added] = generic_instantiations.emplace(inst.clr_name, std::move(inst));
    if (added)
    {
        // First time processing this specialization
        dependent_namespaces.emplace(ref.TypeNamespace());

        state.generic_param_stack.emplace_back(genericArgs.first, genericArgs.second);
        process_dependencies(def.type_def, state);
        state.generic_param_stack.pop_back();
    }

    return type_ref{
        itr->second.clr_name,
        itr->second.mangled_name,
        itr->second.mangled_name
    };
}
