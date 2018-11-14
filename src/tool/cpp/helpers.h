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

        bool has_params() const
        {
            return !m_params.empty();
        }

        bool has_abi_params() const
        {
            return has_params() || return_signature();
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
    static bool has_attribute(T const& row, std::string_view const& type_namespace, std::string_view const& type_name)
    {
        return static_cast<bool>(get_attribute(row, type_namespace, type_name));
    }

    static bool is_fast_class(TypeDef const& type)
    {
        return has_attribute(type, "Windows.Foundation.Metadata", "FastAbiAttribute");
    }

    static bool is_exclusive(TypeDef const& type)
    {
        return has_attribute(type, "Windows.Foundation.Metadata", "ExclusiveToAttribute");
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
            throw_invalid("Type '", type.TypeNamespace(), ".", type.TypeName(), "' does not have a default interface");
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
                w.add_depends(info.type);
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

    struct factory_type
    {
        TypeDef type;
        bool activatable{};
        bool statics{};
        bool composable{};
        bool visible{};
    };

    static auto get_factories(TypeDef const& type)
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

        std::vector<factory_type> factories;

        for (auto&& attribute : type.CustomAttribute())
        {
            auto name = attribute.TypeNamespaceAndName();

            if (name.first == "Windows.Foundation.Metadata")
            {
                auto signature = attribute.Value();

                if (name.second == "ActivatableAttribute")
                {
                    factories.push_back({ get_system_type(signature), true, false });
                }
                else if (name.second == "StaticAttribute")
                {
                    factories.push_back({ get_system_type(signature), false, true });
                }
                else if (name.second == "ComposableAttribute")
                {
                    bool visible{};

                    for (auto&& arg : signature.FixedArgs())
                    {
                        if (auto visibility = std::get_if<ElemSig::EnumValue>(&std::get<ElemSig>(arg.value).value))
                        {
                            visible = std::get<int32_t>(visibility->value) == 2;
                            break;
                        }
                    }

                    factories.push_back({ get_system_type(signature), false, false, true, visible });
                }
            }
        }

        return factories;
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
                auto [ns, name] = attribute.TypeNamespaceAndName();

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

    static bool has_factory_members(TypeDef const& type)
    {
        for (auto&& factory : get_factories(type))
        {
            if (!factory.type || !empty(factory.type.MethodList()))
            {
                return true;
            }
        }

        return false;
    }

    static bool is_composable(TypeDef const& type)
    {
        for (auto&& factory : get_factories(type))
        {
            if (factory.composable)
            {
                return true;
            }
        }

        return false;
    }

    static bool has_composable_constructors(TypeDef const& type)
    {
        for (auto&& factory : get_factories(type))
        {
            if (factory.composable && !empty(factory.type.MethodList()))
            {
                return true;
            }
        }

        return false;
    }
}
