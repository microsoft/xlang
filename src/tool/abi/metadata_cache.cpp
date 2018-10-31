#include "pch.h"

#include "metadata_cache.h"
#include "common.h"

using namespace std::literals;
using namespace xlang::meta::reader;

static element_type const& get_metadata_type(ElementType type)
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

static system_type const& get_system_metadata_type(TypeRef const& ref)
{
    XLANG_ASSERT(ref.TypeNamespace() == system_namespace);
    if (ref.TypeName() == "Guid"sv)
    {
        static system_type const guid_type{ "Guid"sv, "GUID"sv };
        return guid_type;
    }

    xlang::throw_invalid("Unknown type '", ref.TypeName(), "' in System namespace");
}

metadata_cache::metadata_cache(xlang::meta::reader::cache const& c)
{
    xlang::task_group group;

    for (auto const& [ns, members] : c.namespaces())
    {
        // We don't do any synchronization of access to this type's data structures, so reserve space on the "main"
        // thread. Note that set/map iterators are not invalidated on insert/emplace
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
        auto [itr, added] = typeMap.emplace(e.TypeName(), type_def{ this, e });
        XLANG_ASSERT(added);
        target.types.emplace(itr->second);
        target.enums.emplace_back(itr->second);
    }

    target.structs.reserve(members.structs.size());
    for (auto const& s : members.structs)
    {
        auto [itr, added] = typeMap.emplace(s.TypeName(), type_def{ this, s });
        XLANG_ASSERT(added);
        target.types.emplace(itr->second);
        target.structs.emplace_back(itr->second);
    }

    target.delegates.reserve(members.delegates.size());
    for (auto const& d : members.delegates)
    {
        auto [itr, added] = typeMap.emplace(d.TypeName(), type_def{ this, d });
        XLANG_ASSERT(added);
        target.types.emplace(itr->second);
        target.delegates.emplace_back(itr->second);
    }

    target.interfaces.reserve(members.interfaces.size());
    for (auto const& i : members.interfaces)
    {
        auto [itr, added] = typeMap.emplace(i.TypeName(), type_def{ this, i });
        XLANG_ASSERT(added);
        target.types.emplace(itr->second);
        target.interfaces.emplace_back(itr->second);
    }

    target.classes.reserve(members.classes.size());
    for (auto const& c : members.classes)
    {
        auto [itr, added] = typeMap.emplace(c.TypeName(), type_def{ this, c });
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

        target.contracts.insert(api_contract{
                type_name{ contract.TypeNamespace(), contract.TypeName() },
                std::get<uint32_t>(std::get<ElemSig>(attr.Value().FixedArgs()[0].value).value)
            });
    }
}

void type_cache::process_dependencies()
{
    init_state state;
    for (auto const& def : types)
    {
        process_dependencies(def.get().type(), state);
        XLANG_ASSERT(state.parent_generic_inst == nullptr);
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

    // NOTE: We really want 'std::set_union', but since we should only be merging unique sets, merge is more efficient
    std::merge(included_namespaces.begin(), included_namespaces.end(), other.included_namespaces.begin(), other.included_namespaces.end(), std::back_inserter(result.included_namespaces));
    std::merge(enums.begin(), enums.end(), other.enums.begin(), other.enums.end(), std::back_inserter(result.enums));
    std::merge(structs.begin(), structs.end(), other.structs.begin(), other.structs.end(), std::back_inserter(result.structs));
    std::merge(delegates.begin(), delegates.end(), other.delegates.begin(), other.delegates.end(), std::back_inserter(result.delegates));
    std::merge(interfaces.begin(), interfaces.end(), other.interfaces.begin(), other.interfaces.end(), std::back_inserter(result.interfaces));
    std::merge(classes.begin(), classes.end(), other.classes.begin(), other.classes.end(), std::back_inserter(result.classes));

    return result;
}
