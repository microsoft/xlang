#pragma once

namespace xlang
{
    static auto get_start_time()
    {
        return std::chrono::high_resolution_clock::now();
    }

    static auto get_elapsed_time(std::chrono::time_point<std::chrono::high_resolution_clock> const& start)
    {
        return std::chrono::duration_cast<std::chrono::duration<int64_t, std::milli>>(std::chrono::high_resolution_clock::now() - start).count();
    }

    struct method_signature
    {
        using param_t = std::pair<Param, ParamSig const*>;

        explicit method_signature(MethodDef const& method) :
            m_method(method.Signature())
        {
            auto params = method.ParamList();

            if (m_method.ReturnType() && params.first != params.second && params.first.Sequence() == 0)
            {
                m_return = params.first;
                ++params.first;
            }

            for (uint32_t i{}; i != m_method.Params().size(); ++i)
            {
                m_params.emplace_back(params.first + i, m_method.Params().data() + i);
            }
        }

        std::vector<std::pair<Param, ParamSig const*>>& params()
        {
            return m_params;
        }

        std::vector<std::pair<Param, ParamSig const*>> const& params() const
        {
            return m_params;
        }

        auto const& return_signature() const
        {
            return m_method.ReturnType();
        }

        auto return_param_name() const
        {
            std::string_view name;

            if (m_return)
            {
                name = m_return.Name();
            }
            else
            {
                name = "winrt_impl_result";
            }

            return name;
        }

    private:

        MethodDefSig m_method;
        std::vector<std::pair<Param, ParamSig const*>> m_params;
        Param m_return;
    };

    struct separator
    {
        writer& w;
        bool first{ true };

        void operator()()
        {
            if (first)
            {
                first = false;
            }
            else
            {
                w.write(", ");
            }
        }
    };

    template <typename T>
    bool has_attribute(T const& row, std::string_view const& type_namespace, std::string_view const& type_name)
    {
        return static_cast<bool>(get_attribute(row, type_namespace, type_name));
    }

    static bool is_exclusive(TypeDef const& type)
    {
        return has_attribute(type, "Windows.Foundation.Metadata", "ExclusiveToAttribute");
    }

#if 0
    static bool is_fast_class(TypeDef const& type)
    {
        return has_attribute(type, "Windows.Foundation.Metadata", "FastAbiAttribute");
    }

    static coded_index<TypeDefOrRef> get_default_interface(TypeDef const& type)
    {
        auto impls = type.InterfaceImpl();

        for (auto&& impl : impls)
        {
            if (has_attribute(impl, "Windows.Foundation.Metadata", "DefaultAttribute"))
            {
                return impl.Interface();
            }
        }

        if (impls.first != impls.second)
        {
            return impls.first.Interface();
        }

        return {};
    }

    static auto get_abi_name(MethodDef const& method)
    {
        if (auto overload = get_attribute(method, "Windows.Foundation.Metadata", "OverloadAttribute"))
        {
            return std::get<std::string_view>(std::get<ElemSig>(overload.Value().FixedArgs()[0].value).value);
        }
        else
        {
            return method.Name();
        }
    }

    static auto get_name(MethodDef const& method)
    {
        auto name = method.Name();

        if (method.SpecialName())
        {
            return name.substr(name.find('_') + 1);
        }

        return name;
    }

    static bool is_remove_overload(MethodDef const& method)
    {
        return method.SpecialName() && starts_with(method.Name(), "remove_");
    }

    static bool is_add_overload(MethodDef const& method)
    {
        return method.SpecialName() && starts_with(method.Name(), "add_");
    }

    static bool is_put_overload(MethodDef const& method)
    {
        return method.SpecialName() && starts_with(method.Name(), "put_");
    }

    static bool is_noexcept(MethodDef const& method)
    {
        return is_remove_overload(method) || has_attribute(method, "Windows.Foundation.Metadata", "NoExceptAttribute");
    }

    static bool is_async(MethodDef const& method, method_signature const& method_signature)
    {
        if (is_put_overload(method))
        {
            return true;
        }

        if (!method_signature.return_signature())
        {
            return false;
        }

        bool async{};

        call(method_signature.return_signature().Type().Type(),
            [&](GenericTypeInstSig const& type)
        {
            auto generic_type = type.GenericType().TypeRef();

            if (generic_type.TypeNamespace() == "Windows.Foundation")
            {
                auto type_name = generic_type.TypeName();

                async =
                    type_name == "IAsyncAction" ||
                    type_name == "IAsyncOperation`1" ||
                    type_name == "IAsyncActionWithProgress`1" ||
                    type_name == "IAsyncOperationWithProgress`2";
            }
        },
            [](auto&&) {});

        return async;
    }
#endif 

    static TypeDef get_base_class(TypeDef const& derived)
    {
        auto extends = derived.Extends();

        if (!extends)
        {
            return{};
        }

        auto base = extends.TypeRef();
        auto type_name = base.TypeName();
        auto type_namespace = base.TypeNamespace();

        if (type_name == "Object" && type_namespace == "System")
        {
            return {};
        }

        return find_required(base);
    };


    static auto get_bases(TypeDef const& type)
    {
        std::vector<TypeDef> bases;

        for (auto base = get_base_class(type); base; base = get_base_class(base))
        {
            bases.push_back(base);
        }

        return bases;
    }

    struct interface_info
    {
        TypeDef type;
        bool defaulted{};
        bool overridable{};
        bool base{};
        std::vector<std::vector<std::string>> generic_param_stack{};
    };

    static void get_interfaces_impl(writer& w, std::map<std::string, interface_info>& result, bool defaulted, bool overridable, bool base, std::vector<std::vector<std::string>> const& generic_param_stack, std::pair<InterfaceImpl, InterfaceImpl>&& children)
    {
        for (auto&& impl : children)
        {
            interface_info info;
            auto type = impl.Interface();
            auto name = w.write_temp("%", type);
            info.defaulted = !base && (defaulted || has_attribute(impl, "Windows.Foundation.Metadata", "DefaultAttribute"));
            
            {
                // This is for correctness rather than an optimization (but helps performance as well).
                // If the interface was not previously inserted, carry on and recursively insert it.
                // If a previous insertion was defaulted we're done as it is correctly captured.
                // If a newly discovered instance of a previous insertion is not defaulted, we're also done.
                // If it was previously captured as non-defaulted but now found as defaulted, we carry on and
                // rediscover it as we need it to be defaulted recursively.

                auto found = result.find(name);

                if (found != result.end())
                {
                    if (found->second.defaulted || !info.defaulted)
                    {
                        continue;
                    }
                }
            }

            info.overridable = overridable || has_attribute(impl, "Windows.Foundation.Metadata", "OverridableAttribute");
            info.base = base;
            info.generic_param_stack = generic_param_stack;
            writer::generic_param_guard guard;

            switch (type.type())
            {
            case TypeDefOrRef::TypeDef:
            {
                info.type = type.TypeDef();
                break;
            }
            case TypeDefOrRef::TypeRef:
            {
                info.type = find_required(type.TypeRef());
                break;
            }
            case TypeDefOrRef::TypeSpec:
            {
                auto type_signature = type.TypeSpec().Signature();

                std::vector<std::string> names;

                for (auto&& arg : type_signature.GenericTypeInst().GenericArgs())
                {
                    names.push_back(w.write_temp("%", arg));
                }

                info.generic_param_stack.push_back(std::move(names));

                guard = w.push_generic_params(type_signature.GenericTypeInst());
                auto signature = type_signature.GenericTypeInst();
                info.type = find_required(signature.GenericType().TypeRef());

                break;
            }
            }

            get_interfaces_impl(w, result, info.defaulted, info.overridable, base, info.generic_param_stack, info.type.InterfaceImpl());
            result[name] = std::move(info);
        }
    };

    static auto get_interfaces(writer& w, TypeDef const& type)
    {
        std::map<std::string, interface_info> result;
        get_interfaces_impl(w, result, false, false, false, {}, type.InterfaceImpl());

        for (auto&& base : get_bases(type))
        {
            get_interfaces_impl(w, result, false, false, true, {}, base.InterfaceImpl());
        }

        return result;
    }

#if 0
    struct factory_info
    {
        TypeDef type;
        bool activatable{};
        bool statics{};
        bool composable{};
        bool visible{};
    };

    static auto get_factories(writer& w, TypeDef const& type)
    {
        auto get_system_type = [&](auto&& signature) -> TypeDef
        {
            for (auto&& arg : signature.FixedArgs())
            {
                if (auto type_param = std::get_if<ElemSig::SystemType>(&std::get<ElemSig>(arg.value).value))
                {
                    return type.get_cache().find_required(type_param->name);
                }
            }

            return {};
        };

        std::map<std::string, factory_info> result;

        for (auto&& attribute : type.CustomAttribute())
        {
            auto attribute_name = attribute.TypeNamespaceAndName();

            if (attribute_name.first != "Windows.Foundation.Metadata")
            {
                continue;
            }

            auto signature = attribute.Value();
            factory_info info;

            if (attribute_name.second == "ActivatableAttribute")
            {
                info.type = get_system_type(signature);
                info.activatable = true;
            }
            else if (attribute_name.second == "StaticAttribute")
            {
                info.type = get_system_type(signature);
                info.statics = true;
            }
            else if (attribute_name.second == "ComposableAttribute")
            {
                info.type = get_system_type(signature);
                info.composable = true;

                for (auto&& arg : signature.FixedArgs())
                {
                    if (auto visibility = std::get_if<ElemSig::EnumValue>(&std::get<ElemSig>(arg.value).value))
                    {
                        info.visible = std::get<int32_t>(visibility->value) == 2;
                        break;
                    }
                }
            }
            else
            {
                continue;
            }

            std::string name;

            if (info.type)
            {
                name = w.write_temp("%", info.type);
            }

            result[name] = std::move(info);
        }

        return result;
    }

    struct fast_interface_info
    {
        std::string name;
        uint32_t version{};
        std::pair<MethodDef, MethodDef> methods;
    };

    static auto get_fast_interfaces(writer& w, TypeDef const& type)
    {
        w.abi_types = false;

        auto get_version = [](auto&& type)
        {
            for (auto&& attribute : type.CustomAttribute())
            {
                auto[ns, name] = attribute.TypeNamespaceAndName();

                if (ns == "Windows.Foundation.Metadata")
                {
                    if (name == "VersionAttribute")
                    {
                        return std::get<uint32_t>(std::get<ElemSig>(attribute.Value().FixedArgs()[0].value).value);
                    }

                    if (name == "ContractVersionAttribute")
                    {
                        return std::get<uint32_t>(std::get<ElemSig>(attribute.Value().FixedArgs()[1].value).value);
                    }
                }
            }

            return 0u;
        };

        std::vector<fast_interface_info> interfaces;

        for (auto&&[interface_name, interface_info] : get_interfaces(w, type))
        {
            if (!is_exclusive(interface_info.type))
            {
                continue;
            }

            fast_interface_info info;
            info.methods = interface_info.type.MethodList();
            info.name = interface_name;
            info.version = get_version(interface_info.type);

            interfaces.push_back(std::move(info));
        }

        std::sort(interfaces.begin(), interfaces.end(), [](auto&& left, auto&& right)
        {
            return (left.version < right.version || (!(right.version < left.version) && left.name < right.name));
        });

        return interfaces;
    }

    static bool wrap_abi(TypeSig const& signature)
    {
        bool wrap{};

        call(signature.Type(),
            [&](ElementType type)
        {
            wrap = type == ElementType::String || type == ElementType::Object;
        },
            [&](auto&&)
        {
            wrap = true;
        });

        return wrap;
    }

    static bool is_object(TypeSig const& signature)
    {
        bool object{};

        call(signature.Type(),
            [&](ElementType type)
        {
            if (type == ElementType::Object)
            {
                object = true;
            }
        },
            [](auto&&) {});

        return object;
    }

    static auto get_delegate_method(TypeDef const& type)
    {
        auto methods = type.MethodList();

        auto method = std::find_if(begin(methods), end(methods), [](auto&& method)
        {
            return method.Name() == "Invoke";
        });

        if (method == end(methods))
        {
            throw_invalid("Delegate's Invoke method not found");
        }

        return method;
    }

    static std::string get_field_abi(writer& w, Field const& field)
    {
        auto signature = field.Signature();
        auto const& type = signature.Type();
        std::string name = w.write_temp("%", type);

        if (starts_with(name, "struct "))
        {
            auto ref = std::get<coded_index<TypeDefOrRef>>(type.Type());
            XLANG_ASSERT(ref.type() == TypeDefOrRef::TypeRef);

            name = "struct{";

            for (auto&& nested : find_required(ref.TypeRef()).FieldList())
            {
                name += " " + get_field_abi(w, nested) + " ";
                name += nested.Name();
                name += ";";
            }

            name += " }";
        }

        return name;
    }

    static std::string get_component_filename(TypeDef const& type)
    {
        std::string result{ type.TypeNamespace() };
        result += '.';
        result += type.TypeName();

        if (!settings.component_name.empty() && starts_with(result, settings.component_name))
        {
            result = result.substr(settings.component_name.size());

            if (starts_with(result, "."))
            {
                result.erase(result.begin());
            }
        }

        return result;
    }

    static std::string get_generated_component_filename(TypeDef const& type)
    {
        auto result = get_component_filename(type);

        if (!settings.component_prefix)
        {
            std::replace(result.begin(), result.end(), '.', '/');
        }

        return result;
    }

    static bool has_factory_members(writer& w, TypeDef const& type)
    {
        for (auto&&[factory_name, factory] : get_factories(w, type))
        {
            if (!factory.type || !empty(factory.type.MethodList()))
            {
                return true;
            }
        }

        return false;
    }

    static bool is_composable(writer& w, TypeDef const& type)
    {
        for (auto&&[factory_name, factory] : get_factories(w, type))
        {
            if (factory.composable)
            {
                return true;
            }
        }

        return false;
    }

    static bool has_composable_constructors(writer& w, TypeDef const& type)
    {
        for (auto&&[interface_name, factory] : get_factories(w, type))
        {
            if (factory.composable && !empty(factory.type.MethodList()))
            {
                return true;
            }
        }

        return false;
    }
#endif

    // from python helpers

    struct interface_info2
    {
        TypeDef type;
        std::vector<std::string> type_arguments;
    };

    void push_interface_info(std::vector<interface_info2>& interfaces, interface_info2&& info)
    {
        auto iter = std::find_if(interfaces.begin(), interfaces.end(), [&info](auto i)
            {
                return i.type == info.type;
            });

        if (iter == interfaces.end())
        {
            interfaces.push_back(std::move(info));
        }
    }

    void collect_required_interfaces(writer& w, std::vector<interface_info2>& sigs, coded_index<TypeDefOrRef> const& index);

    void collect_required_interfaces(writer& w, std::vector<interface_info2>& interfaces, TypeDef const& type)
    {
        interface_info2 info;
        info.type = type;
        for (auto&& gp : type.GenericParam())
        {
            info.type_arguments.push_back(std::string{ gp.Name() });
        }

        push_interface_info(interfaces, std::move(info));

        auto guard{ w.push_generic_params(type.GenericParam()) };
        for (auto&& ii : type.InterfaceImpl())
        {
            collect_required_interfaces(w, interfaces, ii.Interface());
        }
    }

    void collect_required_interfaces(writer& w, std::vector<interface_info2>& interfaces, GenericTypeInstSig const& sig)
    {
        TypeDef type{};
        switch (sig.GenericType().type())
        {
        case TypeDefOrRef::TypeDef:
            type = sig.GenericType().TypeDef();
            break;
        case TypeDefOrRef::TypeRef:
            type = find_required(sig.GenericType().TypeRef());
            break;
        case TypeDefOrRef::TypeSpec:
            throw_invalid("collect_required_interfaces");
        }

        interface_info2 info;
        info.type = type;

        for (auto&& gp : sig.GenericArgs())
        {
            auto q = w.write_temp("%", gp);
            info.type_arguments.push_back(q);
        }

        push_interface_info(interfaces, std::move(info));

        auto guard{ w.push_generic_params(sig) };
        for (auto&& ii : type.InterfaceImpl())
        {
            collect_required_interfaces(w, interfaces, ii.Interface());
        }
    }

    void collect_required_interfaces(writer& w, std::vector<interface_info2>& sigs, coded_index<TypeDefOrRef> const& index)
    {
        switch (index.type())
        {
        case TypeDefOrRef::TypeDef:
            collect_required_interfaces(w, sigs, index.TypeDef());
            break;
        case TypeDefOrRef::TypeRef:
            collect_required_interfaces(w, sigs, find_required(index.TypeRef()));
            break;
        case TypeDefOrRef::TypeSpec:
            collect_required_interfaces(w, sigs, index.TypeSpec().Signature().GenericTypeInst());
            break;
        }
    }

    auto get_required_interfaces(TypeDef const& type)
    {
        writer w;
        auto guard{ w.push_generic_params(type.GenericParam()) };

        std::vector<interface_info2> interfaces;
        collect_required_interfaces(w, interfaces, type);

        return std::move(interfaces);
    }

    struct method_info
    {
        MethodDef method;
        std::vector<std::string> type_arguments;
    };

    bool is_constructor(MethodDef const& method);

    auto get_methods(TypeDef const& type, bool include_special = false)
    {
        std::map<std::string_view, std::vector<method_info>> method_map{};
        auto category = get_category(type);

        if (category == category::class_type)
        {
            for (auto&& method : type.MethodList())
            {
                if (is_constructor(method) || (method.SpecialName() && !include_special))
                {
                    continue;
                }

                method_map[method.Name()].push_back(method_info{ method, std::vector<std::string> {} });
            }
        }
        else if (category == category::interface_type)
        {
            for (auto&& info : get_required_interfaces(type))
            {
                for (auto&& method : info.type.MethodList())
                {
                    if (method.SpecialName() && !include_special)
                    {
                        continue;
                    }

                    method_map[method.Name()].push_back(method_info{ method, info.type_arguments });
                }
            }
        }
        else
        {
            throw_invalid("only classes and interfaces have methods");
        }

#ifdef XLANG_DEBUG
        for (auto&&[name, method_infos] : method_map)
        {
            XLANG_ASSERT(method_infos.size() > 0);
            auto static_method = method_infos[0].method.Flags().Static();
            for (auto&& info : method_infos)
            {
                XLANG_ASSERT(info.method.Flags().Static() == static_method);
            }
        }
#endif

        return std::move(method_map);
    }

    struct property_info
    {
        Property property;
        std::vector<std::string> type_arguments;
    };

    auto get_properties(TypeDef const& type)
    {
        std::vector<property_info> properties{};

        auto category = get_category(type);
        if (category == category::class_type)
        {
            for (auto&& prop : type.PropertyList())
            {
                properties.push_back(property_info{ prop, std::vector<std::string> {} });
            }
        }
        else if (category == category::interface_type)
        {
            for (auto&& info : get_required_interfaces(type))
            {
                for (auto&& prop : info.type.PropertyList())
                {
                    properties.push_back(property_info{ prop, info.type_arguments });
                }
            }
        }

        return std::move(properties);
    }

    struct event_info
    {
        Event event;
        std::vector<std::string> type_arguments;
    };

    auto get_events(TypeDef const& type)
    {
        std::vector<event_info> events{};

        auto category = get_category(type);
        if (category == category::class_type)
        {
            for (auto&& event : type.EventList())
            {
                events.push_back(event_info{ event, std::vector<std::string> {} });
            }
        }
        else if (category == category::interface_type)
        {
            for (auto&& info : get_required_interfaces(type))
            {
                for (auto&& event : info.type.EventList())
                {
                    events.push_back(event_info{ event, info.type_arguments });
                }
            }
        }

        return std::move(events);
    }

    auto get_constructors(TypeDef const& type)
    {
        std::vector<MethodDef> constructors;

        for (auto&& method : type.MethodList())
        {
            if (is_constructor(method))
            {
                constructors.push_back(method);
            }
        }

        return std::move(constructors);
    }

    auto get_delegate_invoke(TypeDef const& type)
    {
        XLANG_ASSERT(get_category(type) == category::delegate_type);

        for (auto&& method : type.MethodList())
        {
            if (method.SpecialName() && (method.Name() == "Invoke"))
            {
                return method;
            }
        }

        throw_invalid("Invoke method not found");
    }

    bool is_exclusive_to(TypeDef const& type)
    {
        return get_category(type) == category::interface_type && get_attribute(type, "Windows.Foundation.Metadata", "ExclusiveToAttribute");
    }

    bool is_flags_enum(TypeDef const& type)
    {
        return get_category(type) == category::enum_type && get_attribute(type, "System", "FlagsAttribute");
    }

    bool is_ptype(TypeDef const& type)
    {
        return distance(type.GenericParam()) > 0;
    }

    bool is_static(TypeDef const& type)
    {
        return get_category(type) == category::class_type && type.Flags().Abstract();
    }

    std::string_view get_method_abi_name(MethodDef const& method)
    {
        auto overload_attrib = get_attribute(method, "Windows.Foundation.Metadata", "OverloadAttribute");
        if (overload_attrib)
        {
            auto args = overload_attrib.Value().FixedArgs();
            return std::get<std::string_view>(std::get<ElemSig>(args[0].value).value);
        }

        return method.Name();
    }

    bool is_special(MethodDef const& method)
    {
        return method.SpecialName() || method.Flags().RTSpecialName();
    }

    bool is_constructor(MethodDef const& method)
    {
        return method.Flags().RTSpecialName() && method.Name() == ".ctor";
    }

    inline bool is_get_method(MethodDef const& method)
    {
        return method.SpecialName() && starts_with(method.Name(), "get_");
    }

    inline bool is_put_method(MethodDef const& method)
    {
        return method.SpecialName() && starts_with(method.Name(), "put_");
    }

    inline bool is_property_method(MethodDef const& method)
    {
        return method.SpecialName() && (starts_with(method.Name(), "get_") || starts_with(method.Name(), "put_"));
    }

    inline bool is_add_method(MethodDef const& method)
    {
        return method.SpecialName() && starts_with(method.Name(), "add_");
    }

    inline bool is_remove_method(MethodDef const& method)
    {
        return method.SpecialName() && starts_with(method.Name(), "remove_");
    }

    inline bool is_event_method(MethodDef const& method)
    {
        return method.SpecialName() && (starts_with(method.Name(), "add_") || starts_with(method.Name(), "remove_"));
    }

    inline bool is_static(MethodDef const& method)
    {
        return method.Flags().Static();
    }

    struct property_type
    {
        MethodDef get;
        MethodDef set;
    };

    property_type get_property_methods(Property const& prop)
    {
        MethodDef get_method{}, set_method{};

        for (auto&& method_semantic : prop.MethodSemantic())
        {
            auto semantic = method_semantic.Semantic();

            if (semantic.Getter())
            {
                get_method = method_semantic.Method();
            }
            else if (semantic.Setter())
            {
                set_method = method_semantic.Method();
            }
            else
            {
                throw_invalid("Properties can only have get and set methods");
            }
        }

        XLANG_ASSERT(get_method);

        if (set_method)
        {
            XLANG_ASSERT(get_method.Flags().Static() == set_method.Flags().Static());
        }

        return { get_method, set_method };
    }

    bool is_static(Property const& prop)
    {
        auto methods = get_property_methods(prop);
        return is_static(methods.get);
    }

    bool is_static(property_info const& prop)
    {
        return is_static(prop.property);
    }

    struct event_type
    {
        MethodDef add;
        MethodDef remove;
    };

    event_type get_event_methods(Event const& evt)
    {
        MethodDef add_method{}, remove_method{};

        for (auto&& method_semantic : evt.MethodSemantic())
        {
            auto semantic = method_semantic.Semantic();

            if (semantic.AddOn())
            {
                add_method = method_semantic.Method();
            }
            else if (semantic.RemoveOn())
            {
                remove_method = method_semantic.Method();
            }
            else
            {
                throw_invalid("Events can only have add and remove methods");
            }
        }

        XLANG_ASSERT(add_method);
        XLANG_ASSERT(remove_method);
        XLANG_ASSERT(add_method.Flags().Static() == remove_method.Flags().Static());

        return { add_method, remove_method };
    }

    bool is_static(Event const& evt)
    {
        auto methods = get_event_methods(evt);
        return is_static(methods.add);
    }

    bool is_static(event_info const& evt)
    {
        return is_static(evt.event);
    }

    bool has_dealloc(TypeDef const& type)
    {
        auto category = get_category(type);
        return category == category::interface_type || (category == category::class_type && !type.Flags().Abstract());
    }

    TypeSig get_ireference_type(GenericTypeInstSig const& type)
    {
        TypeDef td{ };

        switch (type.GenericType().type())
        {
        case TypeDefOrRef::TypeDef:
            td = type.GenericType().TypeDef();
            break;
        case TypeDefOrRef::TypeRef:
            td = find_required(type.GenericType().TypeRef());
            break;
        default:
            throw_invalid("expecting TypeDef or TypeRef");
        }

        if ((td.TypeNamespace() != "Windows.Foundation") || (td.TypeName() != "IReference`1"))
        {
            throw_invalid("Expecting Windows.Foundation.IReference");
        }

        XLANG_ASSERT(type.GenericArgCount() == 1);

        return *(type.GenericArgs().first);
    }

    bool is_customized_struct(TypeDef const& type)
    {
        if (type.TypeNamespace() == "Windows.Foundation")
        {
            static const std::set<std::string_view> custom_structs = { "DateTime", "EventRegistrationToken", "HResult", "TimeSpan" };

            return custom_structs.find(type.TypeName()) != custom_structs.end();
        }

        return false;
    }

    enum class param_category
    {
        in,
        out,
        pass_array,
        fill_array,
        receive_array,
    };

    auto get_param_category(method_signature::param_t const& param)
    {
        if (param.second->Type().is_szarray())
        {
            if (param.first.Flags().In())
            {
                return param_category::pass_array;
            }
            else if (param.second->ByRef())
            {
                XLANG_ASSERT(param.first.Flags().Out());
                return param_category::receive_array;
            }
            else
            {
                XLANG_ASSERT(param.first.Flags().Out());
                return param_category::fill_array;
            }
        }
        else
        {
            if (param.first.Flags().In())
            {
                XLANG_ASSERT(!param.first.Flags().Out());
                return param_category::in;
            }
            else
            {
                XLANG_ASSERT(param.first.Flags().Out());
                return param_category::out;
            }
        }
    }

    auto get_param_category(RetTypeSig const& sig)
    {
        if (sig.Type().is_szarray())
        {
            return param_category::receive_array;
        }
        else
        {
            return param_category::out;
        }
    }

    bool is_in_param(method_signature::param_t const& param)
    {
        auto category = get_param_category(param);

        return (category == param_category::in
            || category == param_category::pass_array
            || category == param_category::fill_array);
    }

    bool is_out_param(method_signature::param_t const& param)
    {
        auto category = get_param_category(param);

        return (category == param_category::out || category == param_category::receive_array);
    }

    int count_in_param(std::vector<method_signature::param_t> const& params)
    {
        int count{ 0 };

        for (auto&& param : params)
        {
            if (is_in_param(param))
            {
                count++;
            }
        }

        return count;
    }

    int count_out_param(std::vector<method_signature::param_t> const& params)
    {
        int count{ 0 };

        for (auto&& param : params)
        {
            if (is_out_param(param))
            {
                count++;
            }
        }

        return count;
    }
}
