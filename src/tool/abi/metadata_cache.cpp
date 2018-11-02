#include "pch.h"

#include "abi_writer.h"
#include "code_writers.h"
#include "metadata_cache.h"

using namespace std::literals;
using namespace xlang::meta::reader;
using namespace xlang::text;

metadata_cache::metadata_cache(xlang::meta::reader::cache const& c)
{
    // We need to initialize in two phases. The first phase creates the collection of all type defs. The second phase
    // processes dependencies and initializes generic types
    // NOTE: We may only need to do this for a subset of types, but that would introduce a fair amount of complexity and
    //       the runtime cost of processing everything is relatively insignificant
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
            process_namespace_types(members, nsTypes, table);
        });
    }
    group.get();

    for (auto& [ns, nsCache] : namespaces)
    {
        group.add([&]()
        {
            process_namespace_dependencies(nsCache, this);
        });
    }
}

void metadata_cache::process_namespace_types(
    cache::namespace_members const& members,
    namespace_cache& target,
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

void metadata_cache::process_namespace_dependencies(namespace_cache& target)
{
    for (auto& enumType : target.enums)
    {

    }

    for (auto& structType : target.structs)
    {

    }

    for (auto& delegateType : target.delegates)
    {

    }

    for (auto& interfaceType : target.interfaces)
    {

    }

    for (auto& classType : target.classes)
    {

    }
}







template <typename T>
static void merge_into(std::vector<T>& from, std::vector<std::reference_wrapper<T>>& to)
{
    std::vector<std::reference_wrapper<T>> result;
    result.reserve(from.size() + to.size());
    std::merge(from.begin(), from.end(), to.begin(), to.end(), std::back_inserter(result));
    to.swap(result);
}

type_cache metadata_cache::process_namespaces(std::initializer_list<std::string_view> targetNamespaces)
{
    type_cache result{ this };
    result.included_namespaces.insert(result.included_namespaces.end(), targetNamespaces.begin(), targetNamespaces.end());

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
    type_cache::init_state state;
    auto process_types = [&](auto const& vector)
    {
        for (auto const& type : vector)
        {
            result.process_type(type, state);
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
void type_cache::process_contract_dependencies(T const& type)
{
    if (auto attr = contract_attributes(type))
    {
        dependent_namespaces.emplace(decompose_type(attr->type_name).first);
        for (auto const& prevContract : attr->previous_contracts)
        {
            dependent_namespaces.emplace(decompose_type(prevContract.type_name).first);
        }
    }

    if (auto info = is_deprecated(type))
    {
        dependent_namespaces.emplace(decompose_type(info->contract_type).first);
    }
}

void type_cache::process_type(enum_type& type, type_cache::init_state& /*state*/)
{
    // There's no pre-processing that we need to do for enums. Just take note of the namespace dependencies that come
    // from contract version(s)/deprecations
    process_contract_dependencies(type.type());

    for (auto const& field : type.type().FieldList())
    {
        process_contract_dependencies(field);
    }
}

void type_cache::process_type(struct_type& type, type_cache::init_state& state)
{
    process_contract_dependencies(type.type());

    for (auto const& field : type.type().FieldList())
    {
        process_contract_dependencies(field);
        type.members.push_back(struct_member{ field, &find(field.Signature().Type(), state.parent_generic_inst) });
    }
}

function_def type_cache::process_function(MethodDef const& def, type_cache::init_state& state)
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

        return_type = function_return_type{ sig.ReturnType(), name, &find(sig.ReturnType().Type(), state.parent_generic_inst) };
    }

    std::vector<function_param> params;
    for (auto const& param : sig.Params())
    {
        XLANG_ASSERT(paramNames.first != paramNames.second);
        params.push_back(function_param{ param, paramNames.first.Name(), &find(param.Type(), state.parent_generic_inst) });
        ++paramNames.first;
    }

    return function_def{ def, std::move(return_type), std::move(params) };
}

void type_cache::process_type(delegate_type& type, type_cache::init_state& state)
{
    process_contract_dependencies(type.type());

    // We only care about instantiations of generic types, so early exit as we won't be able to resolve references
    if (type.is_generic())
    {
        return;
    }

    // Delegates only have a single function - Invoke - that we care about
    for (auto const& method : type.type().MethodList())
    {
        if (method.Name() != ".ctor"sv)
        {
            XLANG_ASSERT(method.Name() == "Invoke"sv);
            process_contract_dependencies(method);
            type.invoke = process_function(method, state);
            break;
        }
    }
}

void type_cache::process_type(interface_type& type, type_cache::init_state& state)
{
    process_contract_dependencies(type.type());

    // We only care about instantiations of generic types, so early exit as we won't be able to resolve references
    if (type.is_generic())
    {
        return;
    }

    for (auto const& method : type.type().MethodList())
    {
        process_contract_dependencies(method);
        type.functions.push_back(process_function(method, state));
    }

    // TODO: Required interfaces? At least the generic ones?
}

void type_cache::process_type(class_type& type, type_cache::init_state& state)
{
    process_contract_dependencies(type.type());

    // We only care about instantiations of generic types, so early exit as we won't be able to resolve references
    if (type.is_generic())
    {
        return;
    }

    // For classes, all we care about is the interfaces and taking note of the default interface, if there is one
    for (auto const& iface : type.type().InterfaceImpl())
    {
        process_contract_dependencies(iface);
        if (auto attr = get_attribute(iface, metadata_namespace, "DefaultAttribute"sv))
        {
            XLANG_ASSERT(!type.default_interface);
            type.default_interface = &find(iface.Interface(), state.parent_generic_inst);
        }
        // TODO: namespace dependency? Process generic types?
    }
}

metadata_type const* type_cache::try_find(TypeSig const& sig, generic_inst const* parentGenericInst)
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
            result = try_find(t, parentGenericInst);
        },
        [&](GenericTypeIndex t)
        {
            if (parentGenericInst)
            {
                XLANG_ASSERT(t.index < parentGenericInst->generic_params().size());
                result = parentGenericInst->generic_params()[t.index];
            }
            else
            {
                XLANG_ASSERT(false);
            }
        },
        [&](GenericTypeInstSig const& t)
        {
            result = try_find(t, parentGenericInst);
        });

    return result;
}

metadata_type const* type_cache::try_find(coded_index<TypeDefOrRef> const& type, generic_inst const* parentGenericInst)
{
    metadata_type const* result = nullptr;

    visit(type, xlang::visit_overload{
        [&](GenericTypeInstSig const& sig)
        {
            result = try_find(sig, parentGenericInst);
        },
        [&](auto const& defOrRef)
        {
            result = cache->try_find(defOrRef.TypeNamespace(), defOrRef.TypeName());
        }});

    return result;
}

metadata_type const* type_cache::try_find(GenericTypeInstSig const& sig, generic_inst const* parentGenericInst)
{
    metadata_type const* genericType = try_find(sig.GenericType(), parentGenericInst);
    if (!genericType)
    {
        return nullptr;
    }

    std::vector<metadata_type const*> genericParams;
    for (auto const& param : sig.GenericArgs())
    {
        auto ptr = try_find(param, parentGenericInst);
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

void enum_type::write_cpp_forward_declaration(writer& w) const
{
    // TODO
}

void struct_type::write_cpp_forward_declaration(writer& w) const
{
    // TODO
}

void delegate_type::write_cpp_forward_declaration(writer& w) const
{
    if (!w.should_declare(m_mangledName))
    {
        return;
    }

    w.write(R"^-^(#ifndef __%_FWD_DEFINED__
#define __%_FWD_DEFINED__
)^-^", bind<write_mangled_name>(m_mangledName), bind<write_mangled_name>(m_mangledName));

    w.push_namespace(clr_abi_namespace());
    w.write("%interface %;\n", indent{}, m_abiName);
    w.pop_namespace();

    w.write(R"^-^(#define % @::%

#endif // __%_FWD_DEFINED__

)^-^", bind<write_mangled_name>(m_mangledName), clr_abi_namespace(), m_abiName, bind<write_mangled_name>(m_mangledName));
}

void interface_type::write_cpp_forward_declaration(writer& w) const
{
    if (!w.should_declare(m_mangledName))
    {
        return;
    }

    w.write(R"^-^(#ifndef __%_FWD_DEFINED__
#define __%_FWD_DEFINED__
)^-^", bind<write_mangled_name>(m_mangledName), bind<write_mangled_name>(m_mangledName));

    w.push_namespace(clr_abi_namespace());
    w.write("%interface %;\n", indent{}, m_type.TypeName());
    w.pop_namespace();

    w.write(R"^-^(#define % @::%

#endif // __%_FWD_DEFINED__

)^-^", bind<write_mangled_name>(m_mangledName), clr_abi_namespace(), m_type.TypeName(), bind<write_mangled_name>(m_mangledName));
}

void class_type::write_cpp_forward_declaration(writer& w) const
{
    // TODO
}

void generic_inst::write_cpp_forward_declaration(writer& w) const
{
    if (!w.should_declare(m_mangledName))
    {
        return;
    }

    // First need to make sure that all generic parameters are declared
    for (auto param : m_genericParams)
    {
        param->write_cpp_forward_declaration(w);
    }

    w.write(R"^-^(#ifndef DEF_%_USE
#define DEF_%_USE
#if !defined(RO_NO_TEMPLATE_NAME)
)^-^", m_mangledName, m_mangledName);

    w.push_inline_namespace(clr_abi_namespace());

    auto const cppName = generic_type_abi_name();
    w.write(R"^-^(template <>
struct __declspec(uuid("%"))
%<)^-^", "TODO_UUID", cppName);
    // TODO: Generic params
    w.write("> : %_impl<", cppName);
    // TODO: Generic params

    w.write(R"^-^(>
{
    static const wchar_t* z_get_rc_name_impl()
    {
        return L"%";
    }
};
// Define a typedef for the parameterized interface specialization's mangled name.
// This allows code which uses the mangled name for the parameterized interface to access the
// correct parameterized interface specialization.
typedef % %_t;
#define % ABI::Windows::Foundation::Collections::%_t
)^-^", m_clrFullName, "TODO_CPPFULLNAME", m_mangledName, m_mangledName, m_mangledName);

    w.pop_inline_namespace();

    w.write(R"^-^(
#endif // !defined(RO_NO_TEMPLATE_NAME)
#endif /* DEF_%_USE */

)^-^", m_mangledName);
}






#if 0
void type_cache::process_dependencies(TypeDef const& type, type_cache::init_state& state)
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

metadata_type const* type_cache::process_dependency(TypeSig const& type, type_cache::init_state& state)
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

metadata_type const* type_cache::process_dependency(coded_index<TypeDefOrRef> const& type, type_cache::init_state& state)
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

metadata_type const* type_cache::process_dependency(GenericTypeInstSig const& type, type_cache::init_state& state)
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
    static element_type const boolean_type{ "Boolean"sv, "bool"sv, "boolean"sv, "boolean"sv, "b1"sv };
    static element_type const char_type{ "Char16"sv, "wchar_t"sv, "wchar_t"sv, "wchar__zt"sv, "c2"sv };
    static element_type const u1_type{ "UInt8"sv, "::byte"sv, "::byte"sv, "byte"sv, "u1"sv };
    static element_type const i2_type{ "Int16"sv, "short"sv, "short"sv, "short"sv, "i2"sv };
    static element_type const u2_type{ "UInt16"sv, "UINT16"sv, "UINT16"sv, "UINT16"sv, "u2"sv };
    static element_type const i4_type{ "Int32"sv, "int"sv, "int"sv, "int"sv, "i4"sv };
    static element_type const u4_type{ "UInt32"sv, "UINT32"sv, "UINT32"sv, "UINT32"sv, "u4"sv };
    static element_type const i8_type{ "Int64"sv, "__int64"sv, "__int64"sv, "__z__zint64"sv, "i8"sv };
    static element_type const u8_type{ "UInt64"sv, "UINT64"sv, "UINT64"sv, "UINT64"sv, "u8"sv };
    static element_type const r4_type{ "Single"sv, "float"sv, "float"sv, "float"sv, "f4"sv };
    static element_type const r8_type{ "Double"sv, "double"sv, "double"sv, "double"sv, "f8"sv };
    static element_type const string_type{ "String"sv, "HSTRING"sv, "HSTRING"sv, "HSTRING"sv, "string"sv };
    static element_type const object_type{ "Object"sv, "IInspectable*"sv, "IInspectable*"sv, "IInspectable"sv, "cinterface(IInspectable)"sv };

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
        static system_type const guid_type{ "Guid"sv, "GUID"sv, "g16"sv };
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
            static mapped_type const hresult_type{ type, "HRESULT"sv, "struct(Windows.Foundation.HResult;i4)"sv };
            return &hresult_type;
        }
        else if (type.TypeName() == "EventRegistrationToken"sv)
        {
            static mapped_type event_token_type{ type, "EventRegistrationToken"sv, "struct(Windows.Foundation.EventRegistrationToken;i8)"sv };
            return &event_token_type;
        }
        else if (type.TypeName() == "AsyncStatus"sv)
        {
            static mapped_type const async_status_type{ type, "AsyncStatus"sv, "enum(Windows.Foundation.AsyncStatus;i4)"sv };
            return &async_status_type;
        }
        else if (type.TypeName() == "IAsyncInfo"sv)
        {
            static mapped_type const async_info_type{ type, "IAsyncInfo"sv, "{00000036-0000-0000-C000-000000000046}"sv };
            return &async_info_type;
        }
    }

    return nullptr;
}
