#pragma once

namespace xlang
{
    inline auto get_start_time()
    {
        return std::chrono::high_resolution_clock::now();
    }

    inline auto get_elapsed_time(std::chrono::time_point<std::chrono::high_resolution_clock> const& start)
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
                name = "winrt_impl_return_value";
            }

            return name;
        }

        bool has_params() const
        {
            return !m_params.empty();
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

    inline coded_index<TypeDefOrRef> get_default_interface(TypeDef const& type)
    {
        auto impls = type.InterfaceImpl();

        for (auto&& impl : impls)
        {
            if (get_attribute(impl, "Windows.Foundation.Metadata", "DefaultAttribute"))
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

    inline auto get_abi_name(MethodDef const& method)
    {
        std::string_view name;

        if (auto overload = get_attribute(method, "Windows.Foundation.Metadata", "OverloadAttribute"))
        {
            return std::get<std::string_view>(std::get<ElemSig>(overload.Value().FixedArgs()[0].value).value);
        }
        else
        {
            return method.Name();
        }
    }

    inline auto get_name(MethodDef const& method)
    {
        auto name = method.Name();

        if (method.SpecialName())
        {
            return name.substr(name.find('_') + 1);
        }

        return name;
    }

    inline bool is_remove_overload(MethodDef const& method)
    {
        return method.SpecialName() && starts_with(method.Name(), "remove_");
    }

    inline bool is_add_overload(MethodDef const& method)
    {
        return method.SpecialName() && starts_with(method.Name(), "add_");
    }

    inline bool is_put_overload(MethodDef const& method)
    {
        return method.SpecialName() && starts_with(method.Name(), "put_");
    }

    inline bool is_noexcept(MethodDef const& method)
    {
        return is_remove_overload(method) || get_attribute(method, "Experimental.Windows.Foundation.Metadata", "NoExceptAttribute");
    }

    inline bool is_async(MethodDef const& method, method_signature const& method_signature)
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

        visit(method_signature.return_signature().Type().Type(),
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

    inline auto get_bases(TypeDef const& type)
    {
        std::vector<TypeDef> bases;

        auto get_base_class = [](TypeDef const& derived) -> TypeDef
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

        for (auto base = get_base_class(type); base; base = get_base_class(base))
        {
            bases.push_back(base);
        }

        return bases;
    }

    struct interface_info
    {
        coded_index<TypeDefOrRef> type;
        std::pair<MethodDef, MethodDef> methods;
        bool defaulted{};
        bool overridable{};
        bool base{};
    };

    inline void get_interfaces_impl(writer& w, std::map<std::string, interface_info>& result, bool defaulted, bool overridable, bool base, std::pair<InterfaceImpl, InterfaceImpl>&& children)
    {
        for (auto&& impl : children)
        {
            interface_info info{ impl.Interface() };
            auto name = w.write_temp("%", info.type);
            info.defaulted = !base && (defaulted || static_cast<bool>(get_attribute(impl, "Windows.Foundation.Metadata", "DefaultAttribute")));

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

            info.overridable = overridable || static_cast<bool>(get_attribute(impl, "Windows.Foundation.Metadata", "OverridableAttribute"));
            info.base = base;
            TypeDef definition;
            writer::generic_param_guard guard;

            switch (info.type.type())
            {
            case TypeDefOrRef::TypeDef:
            {
                definition = info.type.TypeDef();
                break;
            }
            case TypeDefOrRef::TypeRef:
            {
                definition = find_required(info.type.TypeRef());
                w.add_depends(definition);
                break;
            }
            case TypeDefOrRef::TypeSpec:
            {
                auto type_signature = info.type.TypeSpec().Signature();
                guard = w.push_generic_params(type_signature.GenericTypeInst());
                auto signature = type_signature.GenericTypeInst();
                definition = find_required(signature.GenericType().TypeRef());
                break;
            }
            }

            info.methods = definition.MethodList();
            get_interfaces_impl(w, result, info.defaulted, info.overridable, base, definition.InterfaceImpl());
            result[name] = std::move(info);
        }
    };

    inline auto get_interfaces(writer& w, TypeDef const& type)
    {
        std::map<std::string, interface_info> result;
        get_interfaces_impl(w, result, false, false, false, type.InterfaceImpl());

        for (auto&& base : get_bases(type))
        {
            get_interfaces_impl(w, result, false, false, true, base.InterfaceImpl());
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

    inline auto get_factories(TypeDef const& type)
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

    inline bool wrap_abi(TypeSig const& signature)
    {
        bool wrap{};

        visit(signature.Type(),
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

    inline bool is_object(TypeSig const& signature)
    {
        bool object{};

        visit(signature.Type(),
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

    inline auto get_delegate_method(TypeDef const& type)
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

    inline std::string get_field_abi(writer& w, Field const& field)
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

    inline std::string get_component_filename(TypeDef const& type)
    {
        // TODO: shorten as needed

        std::string result;
        result += type.TypeNamespace();
        result += ".";
        result += type.TypeName();
        return result;
    }
}
