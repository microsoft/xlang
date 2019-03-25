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

    static void get_interfaces_impl(writer& w, std::map<std::string, interface_info>& result, interface_info const& parent, std::pair<InterfaceImpl, InterfaceImpl>&& children)
    {
        for (auto&& impl : children)
        {
            interface_info info;
            auto type = impl.Interface();
            auto name = w.write_temp("%", type);
            info.defaulted = !parent.base && (parent.defaulted || has_attribute(impl, "Windows.Foundation.Metadata", "DefaultAttribute"));
            
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

            info.overridable = parent.overridable || has_attribute(impl, "Windows.Foundation.Metadata", "OverridableAttribute");
            info.base = parent.base;
            info.generic_param_stack = parent.generic_param_stack;
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
                    if(auto type_index = std::get_if<coded_index<TypeDefOrRef>>(&arg.Type()))
                    {
                        if(type_index->type() == TypeDefOrRef::TypeRef)
                        {
                            auto type_ref = type_index->TypeRef();
                            auto type_def = find_required(type_ref);
                            if(type_def.Flags().Semantics() == TypeSemantics::Interface)
                            {
                                //auto exclusive_to = get_exclusive_to(type_def);
                                //if (exclusive_to.has_value())
                                //{
                                //    names.push_back(w.write_temp("%", exclusive_to.value().TypeName()));
                                //    continue;
                                //}
                            }
                        }
                    }
                    names.push_back(w.write_temp("%", arg));
                }

                info.generic_param_stack.push_back(std::move(names));

                guard = w.push_generic_params(type_signature.GenericTypeInst());
                auto signature = type_signature.GenericTypeInst();
                info.type = find_required(signature.GenericType().TypeRef());

                break;
            }
            }

            get_interfaces_impl(w, result, info, info.type.InterfaceImpl());
            result[name] = std::move(info);
        }
    };

    static auto get_interfaces(writer& w, TypeDef const& type)
    {
        struct current_type_guard
        {
            writer& w;
            current_type_guard(writer& w, std::string_view type_name) : w(w)
            {
                w.current_type = type_name;
            }
            ~current_type_guard()
            {
                w.current_type = "";
            }
        }
        current_type(w, type.TypeName());

        std::map<std::string, interface_info> result;
        interface_info parent{};
        get_interfaces_impl(w, result, parent, type.InterfaceImpl());

        parent.base = true;
        for (auto&& base : get_bases(type))
        {
            get_interfaces_impl(w, result, parent, base.InterfaceImpl());
        }

        return result;
    }

    bool is_static(TypeDef const& type)
    {
        return get_category(type) == category::class_type && type.Flags().Abstract();
    }

    inline bool is_static(MethodDef const& method)
    {
        return method.Flags().Static();
    }

    bool is_constructor(MethodDef const& method)
    {
        return method.Flags().RTSpecialName() && method.Name() == ".ctor";
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
}
