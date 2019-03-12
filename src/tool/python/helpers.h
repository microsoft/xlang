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
        std::string_view _separator{ ", " };
        bool first{ true };

        void operator()()
        {
            if (first)
            {
                first = false;
            }
            else
            {
                w.write(_separator);
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

        void handle(GenericTypeInstSig const& type)
        {
            handle(type.GenericType());

            for (auto&& arg : type.GenericArgs())
            {
                handle(arg);
            }
        }

        void handle(ElementType /*type*/) { throw_invalid("handle(ElementType) not implemented"); }

        void handle(GenericTypeIndex /*var*/) { throw_invalid("handle(GenericTypeIndex) not implemented"); }

        void handle(TypeSig const& signature)
        {
            call(signature.Type(), [this](auto&& type) { static_cast<T*>(this)->handle(type); });
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
        if (get_category(type) == category::interface_type)
        {
            interface_info info;
            info.type = type;
            for (auto&& gp : type.GenericParam())
            {
                info.type_arguments.push_back(std::string{ gp.Name() });
            }
            push_interface_info(interfaces, std::move(info));
        }

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

    std::vector<std::string> get_type_arguments(TypeDef const& type, std::string_view const& ns, std::vector<std::string_view> const& names)
    {
        for (auto&& ii : get_required_interfaces(type))
        {
            if (ii.type.TypeNamespace() == ns && std::any_of(
                names.begin(), names.end(), [type_name = ii.type.TypeName()](auto const& name){ return name == type_name; }))
            {
                return std::move(ii.type_arguments);
            }
        }

        return {};
    }

    bool implements_interface(TypeDef const& type, std::string_view const& ns, std::string_view const& name)
    {
        auto category = get_category(type);

        auto type_name_matches = [&ns, &name](TypeDef const& td) { return td.TypeNamespace() == ns && td.TypeName() == name; };

        if (category == category::class_type)
        {
            for (auto&& ii : type.InterfaceImpl())
            {
                switch (ii.Interface().type())
                {
                case TypeDefOrRef::TypeDef:
                {
                    if (implements_interface(ii.Interface().TypeDef(), ns, name))
                    {
                        return true;
                    }
                }
                break;
                case TypeDefOrRef::TypeRef:
                {
                    if (implements_interface(find_required(ii.Interface().TypeRef()), ns, name))
                    {
                        return true;
                    }
                }
                break;
                case TypeDefOrRef::TypeSpec:
                {
                    auto generic_type = ii.Interface().TypeSpec().Signature().GenericTypeInst().GenericType();
                    switch (generic_type.type())
                    {
                    case TypeDefOrRef::TypeDef:
                    {
                        if (implements_interface(generic_type.TypeDef(), ns, name))
                        {
                            return true;
                        }
                    }
                    break;
                    case TypeDefOrRef::TypeRef:
                    {
                        if (implements_interface(find_required(generic_type.TypeRef()), ns, name))
                        {
                            return true;
                        }
                    }
                    break;
                    case TypeDefOrRef::TypeSpec:
                    {
                        throw_invalid("generic type of a type spec can't be a type spec");
                    }
                    break;
                    }
                }
                break;
                }
            }
        }
        else if (category == category::interface_type)
        {
            if (type_name_matches(type))
            {
                return true;
            }

            for (auto&& i : get_required_interfaces(type))
            {
                if (type_name_matches(i.type))
                {
                    return true;
                }
            }
        }

        return false;
    }

    bool implements_any_interface(TypeDef const& type, std::vector<std::tuple<std::string_view, std::string_view>> names)
    {
        for (auto&&[ns, name] : names)
        {
            if (implements_interface(type, ns, name))
            {
                return true;
            }
        }

        return false;
    }

    bool implements_istringable(TypeDef const& type)
    {
        return implements_interface(type, "Windows.Foundation", "IStringable");
    }

    bool implements_iclosable(TypeDef const& type)
    {
        return implements_interface(type, "Windows.Foundation", "IClosable");
    }

    bool is_async_interface(TypeDef const& type)
    {
        return get_category(type) == category::interface_type &&
            implements_any_interface(type, {
                std::make_tuple("Windows.Foundation", "IAsyncAction"),
                std::make_tuple("Windows.Foundation", "IAsyncActionWithProgress`1"),
                std::make_tuple("Windows.Foundation", "IAsyncOperation`1"),
                std::make_tuple("Windows.Foundation", "IAsyncOperationWithProgress`2") });
    }

    bool implements_iiterable(TypeDef const& type)
    {
        return implements_interface(type, "Windows.Foundation.Collections", "IIterable`1");
    }

    bool implements_iiterator(TypeDef const& type)
    {
        return implements_interface(type, "Windows.Foundation.Collections", "IIterator`1");
    }

    bool implements_ivector(TypeDef const& type)
    {
        return implements_interface(type, "Windows.Foundation.Collections", "IVector`1");
    }

    bool implements_ivectorview(TypeDef const& type)
    {
        return implements_interface(type, "Windows.Foundation.Collections", "IVectorView`1");
    }

    bool implements_sequence(TypeDef const& type)
    {
        return implements_ivector(type) || implements_ivectorview(type); 
    }

    bool implements_imap(TypeDef const& type)
    {
        return implements_interface(type, "Windows.Foundation.Collections", "IMap`2");
    }

    bool implements_imapview(TypeDef const& type)
    {
        return implements_interface(type, "Windows.Foundation.Collections", "IMapView`2");
    }

    bool implements_mapping(TypeDef const& type)
    {
        return implements_imap(type) || implements_imapview(type);
    }

    struct method_info
    {
        MethodDef method;
        std::vector<std::string> type_arguments;
    };

    bool is_constructor(MethodDef const& method);

    auto get_member_name(MethodDef const& method)
    {
        auto overload_attrib = get_attribute(method, "Windows.Foundation.Metadata", "OverloadAttribute");
        if (overload_attrib)
        {
            auto args = overload_attrib.Value().FixedArgs();
            return std::get<std::string_view>(std::get<ElemSig>(args[0].value).value);
        }
        else
        {
            return method.Name();
        }
    }

    int count_in_param(std::vector<method_signature::param_t> const& params);

    auto get_members(TypeDef const& type)
    {
        std::map<std::string_view, std::map<int, std::vector<method_info>>> method_map{};
        auto push_func = [&](method_info const& info)
        {
            auto name = get_member_name(info.method);

            method_signature sig{ info.method };
            auto param_count = count_in_param(sig.params());
            method_map[name][param_count].push_back(info);
        };

        auto category = get_category(type);

        if (category == category::class_type)
        {
            for (auto&& method : type.MethodList())
            {
                push_func(method_info{ method, std::vector<std::string> {} });
            }
        }
        else if (category == category::interface_type)
        {
            for (auto&& info : get_required_interfaces(type))
            {
                for (auto&& method : info.type.MethodList())
                {
                    push_func(method_info{ method, info.type_arguments });
                }
            }
        }
        else
        {
            throw_invalid("only classes and interfaces have methods");
        }

#ifdef XLANG_DEBUG
        //for (auto&&[name, method_infos] : method_map)
        //{
        //    XLANG_ASSERT(method_infos.size() > 0);
        //    auto static_method = method_infos[0].method.Flags().Static();
        //    for (auto&& info : method_infos)
        //    {
        //        XLANG_ASSERT(info.method.Flags().Static() == static_method);
        //    }
        //}
#endif

        return std::move(method_map);
    }

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
        return category == category::struct_type ||
            category == category::interface_type || 
            (category == category::class_type && !type.Flags().Abstract());
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
            // Note, fill array acts as in and out param in Python
            || category == param_category::fill_array); 
    }

    bool is_out_param(method_signature::param_t const& param)
    {
        auto category = get_param_category(param);

        return (category == param_category::out 
            || category == param_category::receive_array
            // Note, fill array acts as in and out param in Python
            || category == param_category::fill_array);
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