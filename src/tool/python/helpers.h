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

    auto get_dotted_name_segments(std::string_view ns)
    {
        std::vector<std::string_view> segments;
        size_t pos = 0;

        while (true)
        {
            auto new_pos = ns.find('.', pos);

            if (new_pos == std::string_view::npos)
            {
                segments.push_back(ns.substr(pos));
                return std::move(segments);
            }

            segments.push_back(ns.substr(pos, new_pos - pos));
            pos = new_pos + 1;
        };
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

    bool is_flags_enum(TypeDef const& type);

    template <typename T>
    struct signature_handler_base
    {
        void handle_class(TypeDef const& /*type*/) { throw_invalid("handle_class not implemented"); }
        void handle_delegate(TypeDef const& /*type*/) { throw_invalid("handle_delegate not implemented"); }
        void handle_guid(TypeRef const& /*type*/) { throw_invalid("handle_guid not implemented"); }
        void handle_interface(TypeDef const& /*type*/) { throw_invalid("handle_interface not implemented"); }
        void handle_struct(TypeDef const& /*type*/) { throw_invalid("handle_struct not implemented"); }

        void handle_enum(TypeDef const& type)
        {
            if (is_flags_enum(type))
            {
                static_cast<T*>(this)->handle(ElementType::U4);
            }
            else
            {
                static_cast<T*>(this)->handle(ElementType::I4);
            }
        }

        void handle(TypeRef const& type)
        {
            auto ns = type.TypeNamespace();
            auto name = type.TypeName();

            if (name == "Guid" && ns == "System")
            {
                static_cast<T*>(this)->handle_guid(type);
            }
            else
            {
                static_cast<T*>(this)->handle(find_required(type));
            }
        }

        void handle(TypeDef const& type)
        {
            switch (get_category(type))
            {
            case category::class_type:
                static_cast<T*>(this)->handle_class(type);
                break;
            case category::delegate_type:
                static_cast<T*>(this)->handle_delegate(type);
                break;
            case category::interface_type:
                static_cast<T*>(this)->handle_interface(type);
                break;
            case category::enum_type:
                static_cast<T*>(this)->handle_enum(type);
                break;
            case category::struct_type:
                static_cast<T*>(this)->handle_struct(type);
                break;
            }
        }

        void handle(coded_index<TypeDefOrRef> const& type)
        {
            switch (type.type())
            {
            case TypeDefOrRef::TypeDef:
                static_cast<T*>(this)->handle(type.TypeDef());
                break;

            case TypeDefOrRef::TypeRef:
                static_cast<T*>(this)->handle(type.TypeRef());
                break;

            case TypeDefOrRef::TypeSpec:
                static_cast<T*>(this)->handle(type.TypeSpec().Signature().GenericTypeInst());
                break;
            }
        }

        void handle_start_generic() { throw_invalid("handle_start_generic not implemented"); }

        void handle_end_generic() { throw_invalid("handle_end_generic not implemented"); }

        void handle(GenericTypeInstSig const& type)
        {
            handle(type.GenericType());
            static_cast<T*>(this)->handle_start_generic();
            for (auto&& arg : type.GenericArgs())
            {
                handle(arg);
            }
            static_cast<T*>(this)->handle_end_generic();
        }

        void handle(ElementType /*type*/) { throw_invalid("handle(ElementType) not implemented"); }

        void handle(GenericTypeIndex /*var*/) { throw_invalid("handle(GenericTypeIndex) not implemented"); }

        void handle(TypeSig const& signature)
        {
            xlang::visit(signature.Type(),
                [&](auto&& type)
            {
                static_cast<T*>(this)->handle(type);
            });
        }
    };

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

        std::vector<param_t>& params()
        {
            return m_params;
        }

        std::vector<param_t> const& params() const
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
        std::vector<param_t> m_params;
        Param m_return;
    };

    struct interface_info
    {
        TypeDef type;
        std::vector<std::string> type_arguments;
    };

    void push_interface_info(std::vector<interface_info>& interfaces, interface_info&& info)
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

    void collect_required_interfaces(writer& w, std::vector<interface_info>& sigs, coded_index<TypeDefOrRef> const& index);

    void collect_required_interfaces(writer& w, std::vector<interface_info>& interfaces, TypeDef const& type)
    {
        interface_info info;
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

    void collect_required_interfaces(writer& w, std::vector<interface_info>& interfaces, GenericTypeInstSig const& sig)
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

        interface_info info;
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

    void collect_required_interfaces(writer& w, std::vector<interface_info>& sigs, coded_index<TypeDefOrRef> const& index)
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

        std::vector<interface_info> interfaces;
        collect_required_interfaces(w, interfaces, type);

        return std::move(interfaces);
    }

    struct method_info
    {
        MethodDef method;
        std::vector<std::string> type_arguments;
    };

    bool is_constructor(MethodDef const& method);

    auto get_methods(TypeDef const& type)
    {
        std::map<std::string_view, std::vector<method_info>> method_map{};
        auto category = get_category(type);

        if (category == category::class_type)
        {
            for (auto&& method : type.MethodList())
            {
                if (is_constructor(method))
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

    bool is_static_class(TypeDef const& type)
    {
        return get_category(type) == category::class_type && type.Flags().Abstract();
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

    inline bool is_add_method(MethodDef const& method)
    {
        return method.SpecialName() && starts_with(method.Name(), "add_");
    }

    inline bool is_remove_method(MethodDef const& method)
    {
        return method.SpecialName() && starts_with(method.Name(), "remove_");
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

    bool has_dealloc(TypeDef const& type)
    {
        auto category = get_category(type);
        return category == category::interface_type || (category == category::class_type && !type.Flags().Abstract());
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