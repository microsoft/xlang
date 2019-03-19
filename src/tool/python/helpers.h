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

    enum class fundamental_type
    {
        Boolean = 0x02,
        Char = 0x03,
        I1 = 0x04,
        U1 = 0x05,
        I2 = 0x06,
        U2 = 0x07,
        I4 = 0x08,
        U4 = 0x09,
        I8 = 0x0a,
        U8 = 0x0b,
        R4 = 0x0c,
        R8 = 0x0d,
        String = 0x0e,
    };

    struct object_type{};

    struct guid_type{};

    struct metadata_type
    {
        category category;
        TypeDef type;
    };

    struct generic_type_instance;

    struct generic_type_index
    {
        uint32_t index;
    };

    using signature_handler_type = std::variant<
        fundamental_type,
        object_type,
        guid_type,
        metadata_type,
        generic_type_instance,
        generic_type_index>;

    struct generic_type_instance
    {
        metadata_type generic_type;
        std::vector<signature_handler_type> generic_args{};
    };

    signature_handler_type handle_signature(TypeSig const& signature);

    signature_handler_type handle_signature(GenericTypeInstSig const& type)
    {
        auto generic_type_helper = [&type]()
        {
            switch (type.GenericType().type())
            {
            case TypeDefOrRef::TypeDef:
            {
                auto type_def = type.GenericType().TypeDef();
                return metadata_type{ get_category(type_def) , type_def };
            }
            case TypeDefOrRef::TypeRef:
            {
                auto type_def = find_required(type.GenericType().TypeRef());
                return metadata_type{ get_category(type_def) , type_def };
            }
            }

            throw_invalid("invalid TypeDefOrRef value for GenericTypeInstSig.GenericType");
        };

        auto gti = generic_type_instance{ generic_type_helper() };

        for (auto&& arg : type.GenericArgs())
        {
            gti.generic_args.push_back(handle_signature(arg));
        }

        return gti;
    }

    namespace impl
    {
        template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
        template<class... Ts> overloaded(Ts...)->overloaded<Ts...>;
    }

    signature_handler_type handle_signature(TypeSig const& signature)
    {
        return std::visit(
            impl::overloaded{
            [](ElementType type) -> signature_handler_type
        {
            switch (type)
            {
            case ElementType::Boolean:
            case ElementType::Char:
            case ElementType::I1:
            case ElementType::U1:
            case ElementType::I2:
            case ElementType::U2:
            case ElementType::I4:
            case ElementType::U4:
            case ElementType::I8:
            case ElementType::U8:
            case ElementType::R4:
            case ElementType::R8:
            case ElementType::String:
                return (fundamental_type)type;
            case ElementType::Object:
                return object_type{};
            }

            throw_invalid("element type not supported");
        },
            [](coded_index<TypeDefOrRef> type) -> signature_handler_type
        {
            switch (type.type())
            {
            case TypeDefOrRef::TypeDef:
            {
                auto type_def = type.TypeDef();
                return metadata_type{ get_category(type_def) , type_def };
            }
            case TypeDefOrRef::TypeRef:
            {
                auto type_ref = type.TypeRef();
                if (type_ref.TypeName() == "Guid" && type_ref.TypeNamespace() == "System")
                {
                    return guid_type{};
                }

                auto type_def = find_required(type_ref);
                return metadata_type{ get_category(type_def) , type_def };
            }
            case TypeDefOrRef::TypeSpec:
                return handle_signature(type.TypeSpec().Signature().GenericTypeInst());
            }

            return object_type{};
        },
            [](GenericTypeIndex var) -> signature_handler_type { return generic_type_index{ var.index }; },
            [](GenericTypeInstSig sig) -> signature_handler_type { return handle_signature(sig); }
            }, signature.Type());
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
                name = "_return_value";
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

    inline bool is_constructor(MethodDef const& method)
    {
        return method.Flags().RTSpecialName() && method.Name() == ".ctor";
    }

    inline bool is_static(MethodDef const& method)
    {
        return method.Flags().Static();
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

    std::string_view get_method_abi_name(MethodDef const& method)
    {
        if (is_constructor(method))
        {
            if (empty(method.ParamList()))
            {
                return "_default_ctor";
            }
            else
            {
                method_signature signature{ method };

                auto type = method.Parent();
                for (auto&& attrib : type.CustomAttribute())
                {
                    auto pair = attrib.TypeNamespaceAndName();
                    if (pair.second == "ActivatableAttribute" && pair.first == "Windows.Foundation.Metadata")
                    {
                        auto fixed_args = attrib.Value().FixedArgs();
                        auto elem0 = std::get<ElemSig>(fixed_args[0].value);

                        // if the first param is SystemType, it holds the name of a factory interface
                        if (std::holds_alternative<ElemSig::SystemType>(elem0.value))
                        {
                            auto factory_type = type.get_cache().find_required(
                                std::get<ElemSig::SystemType>(elem0.value).name);

                            for (auto&& factory_method : factory_type.MethodList())
                            {
                                method_signature factory_signature{ factory_method };
                                if (signature.params().size() == factory_signature.params().size())
                                {
                                    // TODO: compare method param types
                                    return get_method_abi_name(factory_method);
                                }
                            }
                        }
                    }
                }

                std::string msg{ "couldn't find factory method " };
                msg.append(type.TypeNamespace());
                msg.append(".");
                msg.append(type.TypeName());
                throw_invalid(msg);
            }
        }
        else
        {
            auto overload_attrib = get_attribute(method, "Windows.Foundation.Metadata", "OverloadAttribute");
            if (overload_attrib)
            {
                auto args = overload_attrib.Value().FixedArgs();
                return std::get<std::string_view>(std::get<ElemSig>(args[0].value).value);
            }

            return method.Name();
        }
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

    enum class argument_convention
    {
        no_args,
        single_arg,
        variable_args
    };

    auto get_argument_convention(MethodDef const& method)
    {
        if (is_constructor(method) && empty(method.ParamList()))
        {
            return argument_convention::no_args;
        }
        else if (is_get_method(method))
        {
            return argument_convention::no_args;
        }
        else if (is_put_method(method) || is_add_method(method) || is_remove_method(method))
        {
            return argument_convention::single_arg;
        }
        else
        {
            return argument_convention::variable_args;
        }
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

    bool has_dealloc(TypeDef const& type)
    {
        auto category = get_category(type);
        return category == category::struct_type ||
            category == category::interface_type ||
            (category == category::class_type && !type.Flags().Abstract());
    }
}