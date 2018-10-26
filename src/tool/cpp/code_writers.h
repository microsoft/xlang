#pragma once

namespace xlang
{
    inline void write_impl_namespace(writer& w)
    {
        w.write("\nnamespace winrt::impl {\n");
    }

    inline void write_std_namespace(writer& w)
    {
        w.write("\nWINRT_EXPORT namespace std {\n");
    }

    inline void write_type_namespace(writer& w, std::string_view const& ns)
    {
        w.write("\nWINRT_EXPORT namespace winrt::@ {\n", ns);
    }

    inline void write_close_namespace(writer& w)
    {
        w.write("\n}\n");
    }

    inline void write_enum_field(writer& w, Field const& field)
    {
        if (auto constant = field.Constant())
        {
            w.write("\n    % = %,",
                field.Name(),
                *constant);
        }
    }

    inline void write_enum(writer& w, TypeDef const& type)
    {
        auto format = R"(
enum class % : %
{%
};
)";

        auto fields = type.FieldList();

        w.write(format,
            type.TypeName(),
            fields.first.Signature().Type(),
            bind_each<write_enum_field>(fields));
    }

    inline void write_forward(writer& w, TypeDef const& type)
    {
        auto type_namespace = type.TypeNamespace();
        auto type_name = type.TypeName();
        auto category = get_category(type);

        if (category == category::enum_type)
        {
            w.write("enum class % : %;\n", type_name, type.FieldList().first.Signature().Type());
        }
        else if ((type_name == "DateTime" || type_name == "TimeSpan") && type_namespace == "Windows.Foundation")
        {
            // Don't forward declare these since they're not structs.
        }
        else
        {
            w.write("struct %;\n", type_name);
        }
    }

    inline void write_enum_flag(writer& w, TypeDef const& type)
    {
        if (get_attribute(type, "System", "FlagsAttribute"))
        {
            w.write("template<> struct is_enum_flag<@::%> : std::true_type {};\n",
                type.TypeNamespace(),
                type.TypeName());
        }
    }

    inline void write_category(writer& w, TypeDef const& type, std::string_view const& category)
    {
        w.write("template <> struct category<%>{ using type = %; };\n",
            type,
            category);
    }

    inline void write_name(writer& w, TypeDef const& type)
    {
        auto ns = type.TypeNamespace();
        auto name = type.TypeName();

        w.write("template <> struct name<@::%>{ static constexpr auto & value{ L\"%.%\" }; };\n",
            ns, name, ns, name);
    }

    inline void write_guid_value(writer& w, std::vector<FixedArgSig> const& args)
    {
        using std::get;

        w.write_printf("0x%08X,0x%04X,0x%04X,{ 0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X }",
            get<uint32_t>(get<ElemSig>(args[0].value).value),
            get<uint16_t>(get<ElemSig>(args[1].value).value),
            get<uint16_t>(get<ElemSig>(args[2].value).value),
            get<uint8_t>(get<ElemSig>(args[3].value).value),
            get<uint8_t>(get<ElemSig>(args[4].value).value),
            get<uint8_t>(get<ElemSig>(args[5].value).value),
            get<uint8_t>(get<ElemSig>(args[6].value).value),
            get<uint8_t>(get<ElemSig>(args[7].value).value),
            get<uint8_t>(get<ElemSig>(args[8].value).value),
            get<uint8_t>(get<ElemSig>(args[9].value).value),
            get<uint8_t>(get<ElemSig>(args[10].value).value));
    }

    inline void write_guid(writer& w, TypeDef const& type)
    {
        auto attribute = get_attribute(type, "Windows.Foundation.Metadata", "GuidAttribute");

        if (!attribute)
        {
            throw_invalid("'Windows.Foundation.Metadata.GuidAttribute' attribute for type '", type.TypeNamespace(), ".", type.TypeName(), "' not found");
        }

        w.write("template <> struct guid_storage<%>{ static constexpr guid value{ % }; };\n",
            type,
            bind<write_guid_value>(attribute.Value().FixedArgs()));
    }

    inline void write_default_interface(writer& w, TypeDef const& type)
    {
        if (auto default_interface = get_default_interface(type))
        {
            w.write("template <> struct default_interface<%>{ using type = %; };\n",
                type,
                default_interface);
        }
    }

    inline void write_struct_category(writer& w, TypeDef const& type)
    {
        w.write("template <> struct category<%>{ using type = struct_category<%>; };\n",
            type,
            bind_list(", ", type.FieldList()));
    }

    inline void write_array_size_name(writer& w, Param const& param)
    {
        if (w.param_names)
        {
            w.write(" __%Size", param.Name());
        }
    }

    inline void write_abi_params(writer& w, method_signature const& method_signature)
    {
        w.abi_types = true;
        separator s{ w };

        for (auto&&[param, param_signature] : method_signature.params())
        {
            s();

            if (param_signature->Type().is_szarray())
            {
                std::string_view format;

                if (param.Flags().In())
                {
                    format = "uint32_t%, %*";
                }
                else if (param_signature->ByRef())
                {
                    format = "uint32_t*%, %**";
                }
                else
                {
                    format = "uint32_t%, %*";
                }

                w.write(format, bind<write_array_size_name>(param), param_signature->Type());
            }
            else
            {
                w.write(param_signature->Type());

                if (param.Flags().In())
                {
                    XLANG_ASSERT(!param.Flags().Out());

                    if (is_const(*param_signature))
                    {
                        w.write(" const&");
                    }
                }
                else
                {
                    XLANG_ASSERT(!param.Flags().In());
                    XLANG_ASSERT(param.Flags().Out());

                    w.write('*');
                }
            }

            if (w.param_names)
            {
                w.write(" %", param.Name());
            }
        }

        if (method_signature.return_signature())
        {
            s();

            auto const& type = method_signature.return_signature().Type();

            if (type.is_szarray())
            {
                w.write("uint32_t* __%Size, %**", method_signature.return_param_name(), type);
            }
            else
            {
                w.write("%*", type);
            }

            if (w.param_names)
            {
                w.write(" %", method_signature.return_param_name());
            }
        }
    }

    inline void write_abi_args(writer& w, method_signature const& method_signature)
    {
        separator s{ w };

        for (auto&&[param, param_signature] : method_signature.params())
        {
            s();
            auto param_name = param.Name();

            if (param_signature->Type().is_szarray())
            {
                std::string_view format;

                if (param.Flags().In())
                {
                    format = "%.size(), get_abi(%)";
                }
                else if (param_signature->ByRef())
                {
                    format = "impl::put_size_abi(%), put_abi(%)";
                }
                else
                {
                    format = "%.size(), get_abi(%)";
                }

                w.write(format, param_name, param_name);
            }
            else
            {
                if (param.Flags().In())
                {
                    XLANG_ASSERT(!param.Flags().Out());

                    if (wrap_abi(param_signature->Type()))
                    {
                        w.write("get_abi(%)", param_name);
                    }
                    else
                    {
                        w.write(param_name);
                    }
                }
                else
                {
                    XLANG_ASSERT(!param.Flags().In());
                    XLANG_ASSERT(param.Flags().Out());

                    if (wrap_abi(param_signature->Type()))
                    {
                        w.write("put_abi(%)", param_name);
                    }
                    else
                    {
                        w.write("&%", param_name);
                    }
                }
            }
        }

        if (method_signature.return_signature())
        {
            s();
            auto const& type = method_signature.return_signature().Type();
            auto param_name = method_signature.return_param_name();

            if (type.is_szarray())
            {
                w.write("impl::put_size_abi(%), put_abi(%)", param_name, param_name);
            }
            else
            {
                if (wrap_abi(method_signature.return_signature().Type()))
                {
                    w.write("put_abi(%)", param_name);
                }
                else
                {
                    w.write("&%", param_name);
                }
            }
        }
    }

    inline void write_abi_declaration(writer& w, MethodDef const& method)
    {
        w.param_names = false;
        method_signature signature{ method };

        w.write("    virtual int32_t WINRT_CALL %(%) noexcept = 0;\n",
            get_abi_name(method),
            bind<write_abi_params>(signature));
    }

    inline void write_interface_abi(writer& w, TypeDef const& type)
    {
        auto format = R"(
template <> struct abi<@::%>{ struct type : IInspectable
{
%};};
)";

        auto guard{ w.push_generic_params(type.GenericParam()) };

        w.write(format,
            type.TypeNamespace(),
            type.TypeName(),
            bind_each<write_abi_declaration>(type.MethodList()));
    }

    inline void write_delegate_abi(writer& w, TypeDef const& type)
    {
        auto format = R"(
template <> struct abi<@::%>{ struct type : IUnknown
{
%};};
)";

        auto guard{ w.push_generic_params(type.GenericParam()) };

        w.write(format,
            type.TypeNamespace(),
            type.TypeName(),
            bind<write_abi_declaration>(get_delegate_method(type)));
    }

    inline void write_field_abi(writer& w, Field const& field)
    {
        w.write("    % %;\n", get_field_abi(w, field), field.Name());
    }

    inline void write_struct_abi(writer& w, TypeDef const& type)
    {
        w.abi_types = true;

        auto format = R"(
struct struct_%
{
%};
template <> struct abi<@::%>{ using type = struct_%; };
)";

        auto type_name = type.TypeName();
        auto type_namespace = type.TypeNamespace();
        auto impl_name = get_impl_name(type_namespace, type_name);

        w.write(format,
            impl_name,
            bind_each<write_field_abi>(type.FieldList()),
            type_namespace,
            type_name,
            impl_name);

    }

    inline void write_consume_params(writer& w, method_signature const& signature)
    {
        separator s{ w };

        for (auto&&[param, param_signature] : signature.params())
        {
            s();

            if (param_signature->Type().is_szarray())
            {
                std::string_view format;

                if (param.Flags().In())
                {
                    format = "array_view<% const>";
                }
                else if (param_signature->ByRef())
                {
                    format = "com_array<%>&";
                }
                else
                {
                    format = "array_view<%>";
                }

                w.write(format, param_signature->Type().Type());
            }
            else
            {
                if (param.Flags().In())
                {
                    XLANG_ASSERT(!param.Flags().Out());
                    w.consume_types = true;

                    auto param_type = std::get_if<ElementType>(&param_signature->Type().Type());

                    if (param_type && *param_type != ElementType::String && *param_type != ElementType::Object)
                    {
                        w.write("%", param_signature->Type());
                    }
                    else
                    {
                        w.write("% const&", param_signature->Type());
                    }

                    w.consume_types = false;
                }
                else
                {
                    XLANG_ASSERT(!param.Flags().In());
                    XLANG_ASSERT(param.Flags().Out());

                    w.write("%&", param_signature->Type());
                }
            }

            w.write(" %", param.Name());
        }
    }

    inline void write_implementation_params(writer& w, method_signature const& method_signature)
    {
        separator s{ w };

        for (auto&&[param, param_signature] : method_signature.params())
        {
            s();

            if (param_signature->Type().is_szarray())
            {
                std::string_view format;

                if (param.Flags().In())
                {
                    format = "array_view<% const>";
                }
                else if (param_signature->ByRef())
                {
                    format = "com_array<%>&";
                }
                else
                {
                    format = "array_view<%>";
                }

                w.write(format, param_signature->Type().Type());
            }
            else
            {
                if (param.Flags().In())
                {
                    XLANG_ASSERT(!param.Flags().Out());

                    auto param_type = std::get_if<ElementType>(&param_signature->Type().Type());

                    if (w.async_types || (param_type && *param_type != ElementType::String && *param_type != ElementType::Object))
                    {
                        w.write("%", param_signature->Type());
                    }
                    else
                    {
                        w.write("% const&", param_signature->Type());
                    }
                }
                else
                {
                    XLANG_ASSERT(!param.Flags().In());
                    XLANG_ASSERT(param.Flags().Out());

                    w.write("%&", param_signature->Type());
                }
            }

            if (w.param_names)
            {
                w.write(" %", param.Name());
            }
        }
    }

    inline void write_consume_declaration(writer& w, MethodDef const& method)
    {
        method_signature signature{ method };
        w.async_types = is_async(method, signature);
        auto method_name = get_name(method);
        auto type = method.Parent();

        w.write("    % %(%) const%;\n",
            signature.return_signature(),
            method_name,
            bind<write_consume_params>(signature),
            is_noexcept(method) ? " noexcept" : "");

        if (is_add_overload(method))
        {
            auto format = R"(    using %_revoker = impl::event_revoker<%, &impl::abi_t<%>::remove_%>;
    %_revoker %(auto_revoke_t, %) const;
)";

            w.write(format,
                method_name,
                type,
                type,
                method_name,
                method_name,
                method_name,
                bind<write_consume_params>(signature));
        }
    }

    inline void write_consume_return_type(writer& w, method_signature const& signature)
    {
        if (signature.return_signature())
        {
            std::string_view format;
            auto return_type = std::get_if<coded_index<TypeDefOrRef>>(&signature.return_signature().Type().Type());
            bool is_class{ false };

            if (return_type)
            {
                if (return_type->type() == TypeDefOrRef::TypeRef)
                {
                    if (auto definition = find(return_type->TypeRef()))
                    {
                        is_class = get_category(definition) == category::class_type;
                    }
                }
                else if (return_type->type() == TypeDefOrRef::TypeDef)
                {
                    is_class = get_category(return_type->TypeDef()) == category::class_type;
                }
            }

            if (is_class)
            {
                format = "\n    % %{ nullptr };";
            }
            else
            {
                format = "\n    % %{};";
            }

            w.write(format, signature.return_signature(), signature.return_param_name());
        }
    }

    inline void write_consume_return_statement(writer& w, method_signature const& signature)
    {
        if (signature.return_signature())
        {
            w.write("\n    return %;", signature.return_param_name());
        }
    }

    inline void write_consume_args(writer& w, method_signature const& signature)
    {
        separator s{ w };

        for (auto&&[param, param_signature] : signature.params())
        {
            s();
            w.write(param.Name());
        }
    }

    inline void write_consume_definitions(writer& w, TypeDef const& type)
    {
        auto guard{ w.push_generic_params(type.GenericParam()) };
        auto type_name = type.TypeName();
        auto type_namespace = type.TypeNamespace();
        auto type_impl_name = get_impl_name(type_namespace, type_name);

        for (auto&& method : type.MethodList())
        {
            auto method_name = get_name(method);
            method_signature signature{ method };
            w.async_types = is_async(method, signature);

            std::string_view format;

            if (is_noexcept(method))
            {
                format = R"(
template <typename D> % consume_%<D>::%(%) const noexcept
{%
    WINRT_VERIFY_(0, WINRT_SHIM(@::%)->%(%));%
}
)";
            }
            else
            {
                format = R"(
template <typename D> % consume_%<D>::%(%) const
{%
    check_hresult(WINRT_SHIM(@::%)->%(%));%
}
)";
            }

            w.write(format,
                signature.return_signature(),
                type_impl_name,
                method_name,
                bind<write_consume_params>(signature),
                bind<write_consume_return_type>(signature),
                type_namespace,
                type_name,
                get_abi_name(method),
                bind<write_abi_args>(signature),
                bind<write_consume_return_statement>(signature));

            if (is_add_overload(method))
            {
                format = R"(
template <typename D> typename consume_%<D>::%_revoker consume_%<D>::%(auto_revoke_t, %) const
{
    return impl::make_event_revoker<D, %_revoker>(this, %(%));
}
)";

                w.write(format,
                    type_impl_name,
                    method_name,
                    type_impl_name,
                    method_name,
                    bind<write_consume_params>(signature),
                    method_name,
                    method_name,
                    bind<write_consume_args>(signature));
            }
        }
    }

    inline void write_consume(writer& w, TypeDef const& type)
    {
        auto format = R"(
template <typename D>
struct consume_%
{
%%};
template <> struct consume<@::%> { template <typename D> using type = consume_%<D>; };
)";

        w.abi_types = false;
        auto guard{ w.push_generic_params(type.GenericParam()) };
        auto type_name = type.TypeName();
        auto type_namespace = type.TypeNamespace();
        auto impl_name = get_impl_name(type_namespace, type_name);

        w.write(format,
            impl_name,
            bind_each<write_consume_declaration>(type.MethodList()),
            "", // TODO: extensions...
            type_namespace,
            type_name,
            impl_name);
    }

    inline void write_produce_params(writer& w, method_signature const& signature)
    {
        w.param_names = true;
        write_abi_params(w, signature);
    }

    inline void write_produce_cleanup_param(writer& w, TypeSig const& signature, std::string_view const& param_name, bool out)
    {
        if (signature.is_szarray())
        {
            auto format = R"(
            *__%Size = 0;
            *% = nullptr;)";

            w.write(format,
                param_name,
                param_name);

            return;
        }

        bool clear{};
        bool optional{};

        xlang::visit(signature.Type(),
            [&](ElementType type)
        {
            if (out && type == ElementType::Object)
            {
                optional = true;
            }
            else if (type == ElementType::String || type == ElementType::Object)
            {
                clear = true;
            }
        },
            [&](coded_index<TypeDefOrRef> const& index)
        {
            XLANG_ASSERT(index.type() == TypeDefOrRef::TypeDef || index.type() == TypeDefOrRef::TypeRef);

            TypeDef type;

            if (index.type() == TypeDefOrRef::TypeDef)
            {
                type = index.TypeDef();
            }
            else if (index.type() == TypeDefOrRef::TypeRef)
            {
                type = find(index.TypeRef());
            }

            if (type)
            {
                auto category = get_category(type);

                clear = category == category::class_type || category == category::interface_type || category == category::delegate_type;
            }
        },
            [&](GenericTypeInstSig const&)
        {
            clear = true;
        },
            [](auto&&) {});

        if (optional)
        {
            auto format = R"(
            if (%) *% = nullptr;
            Windows::Foundation::IInspectable winrt_impl_%;)";

            w.write(format, param_name, param_name, param_name);
        }
        else if (clear)
        {
            auto format = R"(
            *% = nullptr;)";

            w.write(format, param_name);
        }
    }

    inline void write_produce_cleanup(writer& w, method_signature const& method_signature)
    {
        for (auto&&[param, param_signature] : method_signature.params())
        {
            if (param.Flags().In() || !param_signature->ByRef())
            {
                continue;
            }

            write_produce_cleanup_param(w, param_signature->Type(), param.Name(), true);
        }

        if (method_signature.return_signature())
        {
            write_produce_cleanup_param(w, method_signature.return_signature().Type(), method_signature.return_param_name(), false);
        }
    }

    inline void write_produce_assert_params(writer& w, method_signature const& method_signature)
    {
        w.param_names = false;

        if (method_signature.has_params())
        {
            w.write(", ");
            write_implementation_params(w, method_signature);
        }
    }

    inline void write_produce_assert(writer& w, MethodDef const& method, method_signature const& signature)
    {
        w.abi_types = false;
        w.async_types = false;

        w.write("WINRT_ASSERT_DECLARATION(%, WINRT_WRAP(%)%);",
            get_name(method),
            signature.return_signature(),
            bind<write_produce_assert_params>(signature));
    }

    inline void write_produce_args(writer& w, method_signature const& method_signature)
    {
        separator s{ w };

        for (auto&&[param, param_signature] : method_signature.params())
        {
            s();
            auto param_name = param.Name();
            auto param_type = w.write_temp("%", param_signature->Type().Type());

            if (param_signature->Type().is_szarray())
            {
                if (param.Flags().In())
                {
                    w.write("array_view<@ const>(reinterpret_cast<@ const *>(%), reinterpret_cast<@ const *>(%) + __%Size)",
                        param_type,
                        param_type,
                        param_name,
                        param_type,
                        param_name,
                        param_name);
                }
                else if (param_signature->ByRef())
                {
                    w.write("detach_abi<@>(__%Size, %)",
                        param_type,
                        param_name,
                        param_name);
                }
                else
                {
                    w.write("array_view<@>(reinterpret_cast<@*>(%), reinterpret_cast<@*>(%) + __%Size)",
                        param_type,
                        param_type,
                        param_name,
                        param_type,
                        param_name,
                        param_name);
                }
            }
            else
            {
                if (param.Flags().In())
                {
                    if (wrap_abi(param_signature->Type()))
                    {
                        w.write("*reinterpret_cast<% const*>(&%)",
                            param_type,
                            param_name);
                    }
                    else
                    {
                        w.write(param_name);
                    }
                }
                else
                {
                    if (is_object(param_signature->Type()))
                    {
                        w.write("winrt_impl_%", param_name);
                    }
                    else if (wrap_abi(param_signature->Type()))
                    {
                        w.write("*reinterpret_cast<@*>(%)",
                            param_type,
                            param_name);
                    }
                    else
                    {
                        w.write("*%", param_name);
                    }
                }
            }
        }
    }

    inline void write_produce_upcall(writer& w, MethodDef const& method, method_signature const& method_signature)
    {
        if (method_signature.return_signature())
        {
            auto name = method_signature.return_param_name();

            if (method_signature.return_signature().Type().is_szarray())
            {
                w.write("std::tie(*__%Size, *%) = detach_abi(this->shim().%(%));",
                    name,
                    name,
                    get_name(method),
                    bind<write_produce_args>(method_signature));
            }
            else
            {
                w.write("*% = detach_from<%>(this->shim().%(%));",
                    name,
                    method_signature.return_signature(),
                    get_name(method),
                    bind<write_produce_args>(method_signature));
            }
        }
        else
        {
            w.write("this->shim().%(%);",
                get_name(method),
                bind<write_produce_args>(method_signature));
        }

        for (auto&&[param, param_signature] : method_signature.params())
        {
            if (param.Flags().Out() && !param_signature->Type().is_szarray() && is_object(param_signature->Type()))
            {
                auto param_name = param.Name();

                w.write("\n            if (%) *% = detach_abi(winrt_impl_%);", param_name, param_name, param_name);
            }
        }
    }

    inline void write_delegate_upcall(writer& w, method_signature const& method_signature)
    {
        w.abi_types = false;

        if (method_signature.return_signature())
        {
            auto name = method_signature.return_param_name();

            if (method_signature.return_signature().Type().is_szarray())
            {
                w.write("std::tie(*__%Size, *%) = detach_abi((*this)(%))",
                    name,
                    name,
                    bind<write_produce_args>(method_signature));
            }
            else
            {
                w.write("*% = detach_from<%>((*this)(%))",
                    name,
                    method_signature.return_signature(),
                    bind<write_produce_args>(method_signature));
            }
        }
        else
        {
            w.write("(*this)(%)",
                bind<write_produce_args>(method_signature));
        }
    }

    inline void write_produce_method(writer& w, MethodDef const& method)
    {
        std::string_view format;

        if (is_noexcept(method))
        {
            format = R"(
    int32_t WINRT_CALL %(%) noexcept final
    {%
        typename D::abi_guard guard(this->shim());
        %
        %
        return 0;
    }
)";
        }
        else
        {
            format = R"(
    int32_t WINRT_CALL %(%) noexcept final
    {
        try
        {%
            typename D::abi_guard guard(this->shim());
            %
            %
            return 0;
        }
        catch (...) { return to_hresult(); }
    }
)";
        }

        method_signature signature{ method };
        w.async_types = is_async(method, signature);

        w.write(format,
            get_abi_name(method),
            bind<write_produce_params>(signature),
            bind<write_produce_cleanup>(signature),
            bind<write_produce_assert>(method, signature),
            bind<write_produce_upcall>(method, signature));
    }

    inline void write_produce(writer& w, TypeDef const& type)
    {
        auto format = R"(
template <typename D>
struct produce<D, %> : produce_base<D, %>
{%};
)";

        auto guard{ w.push_generic_params(type.GenericParam()) };

        w.write(format,
            type,
            type,
            bind_each<write_produce_method>(type.MethodList()));
    }

    inline void write_dispatch_overridable_method(writer& w, MethodDef const& method)
    {
        auto format = R"(    % %(%)
    {
        if (auto overridable = this->shim_overridable())
        {
            return overridable.%(%);
        }

        return this->shim().%(%);
    }
)";

        method_signature signature{ method };

        w.write(format,
            signature.return_signature(),
            get_name(method),
            bind<write_implementation_params>(signature),
            get_name(method),
            bind<write_consume_args>(signature),
            get_name(method),
            bind<write_consume_args>(signature));
    }

    inline void write_dispatch_overridable(writer& w, TypeDef const& class_type)
    {
        auto format = R"(
template <typename T, typename D>
struct WINRT_EBO produce_dispatch_to_overridable<T, D, %>
    : produce_dispatch_to_overridable_base<T, D, %>
{
%};)";

        for (auto&&[interface_name, info] : get_interfaces(w, class_type))
        {
            if (info.overridable && !info.base)
            {
                w.write(format,
                    interface_name,
                    interface_name,
                    bind_each<write_dispatch_overridable_method>(info.methods));
            }
        }
    }

    inline void write_interface_override_method(writer& w, MethodDef const& method, std::string_view const& interface_name)
    {
        auto format = R"(
template <typename D> % %T<D>::%(%) const
{
    return shim().template try_as<%>().%(%);
}
)";

        method_signature signature{ method };
        auto method_name = get_name(method);

        w.write(format,
            signature.return_signature(),
            interface_name,
            method_name,
            bind<write_consume_params>(signature),
            interface_name,
            method_name,
            bind<write_consume_args>(signature));
    }

    inline void write_interface_override_methods(writer& w, TypeDef const& class_type)
    {
        for (auto&&[interface_name, info] : get_interfaces(w, class_type))
        {
            if (info.overridable && !info.base)
            {
                auto name = info.type.TypeRef().TypeName();

                w.write_each<write_interface_override_method>(info.methods, name);
            }
        };
    }

    inline void write_class_override_implements(writer& w, std::map<std::string, interface_info> const& interfaces)
    {
        bool found{};

        for (auto&&[name, info] : interfaces)
        {
            if (info.overridable)
            {
                w.write(", %", name);
                found = true;
            }
        }

        if (!found)
        {
            w.write(", Windows::Foundation::IInspectable");
        }
    }

    inline void write_class_override_requires(writer& w, std::map<std::string, interface_info> const& interfaces)
    {
        bool found{};

        for (auto&&[name, info] : interfaces)
        {
            if (!info.overridable)
            {
                w.write(", %", name);
                found = true;
            }
        }
    }

    inline void write_class_override_defaults(writer& w, std::map<std::string, interface_info> const& interfaces)
    {
        bool first{ true };

        for (auto&&[name, info] : interfaces)
        {
            if (!info.overridable)
            {
                continue;
            }

            if (first)
            {
                first = false;
                w.write(",\n    %T<D>", name);
            }
            else
            {
                w.write(", %T<D>", name);
            }
        }
    }

    inline void write_class_override_bases(writer& w, TypeDef const& type)
    {
        for (auto&& base : get_bases(type))
        {
            w.write(", %", base);
        }
    }

    inline void write_class_override_constructors(writer& w, std::string_view const& type_name, std::vector<factory_type> const& factories)
    {
        auto format = R"(    %T(%)
    {
        impl::call_factory<%, %>([&](auto&& f) { f.%(%%*this, this->m_inner); });
    }
)";

        for (auto&& factory : factories)
        {
            if (!factory.composable)
            {
                continue;
            }

            for (auto&& method : factory.type.MethodList())
            {
                method_signature signature{ method };
                auto& params = signature.params();
                params.resize(params.size() - 2);

                w.write(format,
                    type_name,
                    bind<write_consume_params>(signature),
                    type_name,
                    factory.type,
                    get_name(method),
                    bind<write_consume_args>(signature),
                    signature.params().empty() ? "" : ", ");
            }
        }
    }

    inline void write_interface_override(writer& w, TypeDef const& type)
    {
        auto format = R"(
template <typename D>
class %T
{
    D& shim() noexcept { return *static_cast<D*>(this); }
    D const& shim() const noexcept { return *static_cast<const D*>(this); }

public:

    using % = winrt::@::%;

%};
)";

        for (auto&&[interface_name, info] : get_interfaces(w, type))
        {
            if (info.overridable && !info.base)
            {
                auto interface_type = find(info.type.TypeRef());
                auto interface_type_name = interface_type.TypeName();

                w.write(format,
                    interface_type_name,
                    interface_type_name,
                    interface_type.TypeNamespace(),
                    interface_type_name,
                    bind_each<write_consume_declaration>(interface_type.MethodList()));
            }
        }
    }

    inline void write_class_override(writer& w, TypeDef const& type)
    {
        auto factories = get_factories(type);
        bool has_composable_factories{};

        for (auto&& factory : factories)
        {
            if (factory.composable && !empty(factory.type.MethodList()))
            {
                has_composable_factories = true;
                break;
            }
        }

        if (!has_composable_factories)
        {
            return;
        }

        auto format = R"(
template <typename D, typename... Interfaces>
struct %T :
    implements<D%, composing, Interfaces...>,
    impl::require<D%>,
    impl::base<D, %%>%
{
    using composable = %;

protected:
%};
)";

        auto type_name = type.TypeName();
        auto interfaces = get_interfaces(w, type);

        w.write(format,
            type_name,
            bind<write_class_override_implements>(interfaces),
            bind<write_class_override_requires>(interfaces),
            type_name,
            bind<write_class_override_bases>(type),
            bind<write_class_override_defaults>(interfaces),
            type_name,
            bind<write_class_override_constructors>(type_name, factories));
    }

    inline void write_interface_requires(writer& w, TypeDef const& type)
    {
        auto interfaces = get_interfaces(w, type);

        if (interfaces.empty())
        {
            return;
        }

        w.write(",\n    impl::require<%", type.TypeName());

        for (auto&&[name, info] : interfaces)
        {
            w.write(", %", name);
        }

        w.write('>');
    }

    inline void write_interface_usings(writer& w, TypeDef const& type)
    {
        auto type_name = type.TypeName();
        auto interfaces_plus_self = get_interfaces(w, type);
        interfaces_plus_self[std::string{ type_name }] = interface_info{ type.coded_index<TypeDefOrRef>(), type.MethodList() };
        std::map<std::string_view, std::set<std::string>> method_usage;

        for (auto&&[interface_name, info] : interfaces_plus_self)
        {
            for (auto&& method : info.methods)
            {
                method_usage[get_name(method)].insert(interface_name);
            }
        }

        for (auto&&[method_name, interfaces] : method_usage)
        {
            if (interfaces.size() <= 1)
            {
                continue;
            }

            for (auto&& interface_name : interfaces)
            {
                w.write("    using impl::consume_t<%, %>::%;\n",
                    type_name,
                    interface_name,
                    method_name);
            }
        }
    }

    inline void write_class_usings(writer& w, TypeDef const& type)
    {
        auto type_name = type.TypeName();
        auto default_interface = get_default_interface(type);
        auto default_interface_name = w.write_temp("%", default_interface);
        std::map<std::string_view, std::set<std::string>> method_usage;

        for (auto&&[interface_name, info] : get_interfaces(w, type))
        {
            if (info.defaulted && !info.base)
            {
                for (auto&& method : info.methods)
                {
                    method_usage[get_name(method)].insert(default_interface_name);
                }
            }
            else
            {
                for (auto&& method : info.methods)
                {
                    method_usage[get_name(method)].insert(interface_name);
                }
            }
        }

        for (auto&&[method_name, interfaces] : method_usage)
        {
            if (interfaces.size() <= 1)
            {
                continue;
            }

            for (auto&& interface_name : interfaces)
            {
                if (default_interface_name == interface_name)
                {
                    w.write("    using %::%;\n",
                        interface_name,
                        method_name);
                }
                else
                {
                    w.write("    using impl::consume_t<%, %>::%;\n",
                        type_name,
                        interface_name,
                        method_name);
                }
            }
        }

    }

    inline void write_interface(writer& w, TypeDef const& type)
    {
        auto format = R"(
struct WINRT_EBO % :
    Windows::Foundation::IInspectable,
    impl::consume_t<%>%
{
    %(std::nullptr_t = nullptr) noexcept {}
%};
)";

        auto type_name = type.TypeName();

        auto guard{ w.push_generic_params(type.GenericParam()) };

        w.write(format,
            type_name,
            type_name,
            bind<write_interface_requires>(type),
            type_name,
            bind<write_interface_usings>(type));
    }

    inline void write_delegate(writer& w, TypeDef const& type)
    {
        auto format = R"(
struct % : Windows::Foundation::IUnknown
{
    %(std::nullptr_t = nullptr) noexcept {}
    template <typename L> %(L lambda);
    template <typename F> %(F* function);
    template <typename O, typename M> %(O* object, M method);
    template <typename O, typename M> %(com_ptr<O>&& object, M method);
    template <typename O, typename M> %(weak_ref<O>&& object, M method);
    % operator()(%) const;
};
)";

        auto type_name = type.TypeName();
        auto guard{ w.push_generic_params(type.GenericParam()) };
        method_signature signature{ get_delegate_method(type) };

        w.write(format,
            type_name,
            type_name,
            type_name,
            type_name,
            type_name,
            type_name,
            type_name,
            signature.return_signature(),
            bind<write_consume_params>(signature));
    }

    inline void write_delegate_implementation(writer& w, TypeDef const& type)
    {
        auto format = R"(
template <> struct delegate<@::%>
{
    template <typename H>
    struct type : implements_delegate<@::%, H>
    {
        type(H&& handler) : implements_delegate<@::%, H>(std::forward<H>(handler)) {}

        int32_t WINRT_CALL Invoke(%) noexcept final
        {
            try
            {
                %;
                return 0;
            }
            catch (...)
            {%
                return to_hresult();
            }
        }
    };
};
)";

        w.param_names = true;
        auto guard{ w.push_generic_params(type.GenericParam()) };
        auto type_name = type.TypeName();
        auto type_namespace = type.TypeNamespace();
        method_signature signature{ get_delegate_method(type) };

        w.write(format,
            type_namespace, type_name,
            type_namespace, type_name,
            type_namespace, type_name,
            bind<write_abi_params>(signature),
            bind<write_delegate_upcall>(signature),
            "");
    }

    inline void write_delegate_definition(writer& w, TypeDef const& type)
    {
        auto format = R"(
template <typename L> %::%(L handler) :
    %(impl::make_delegate<%>(std::forward<L>(handler)))
{}

template <typename F> %::%(F* handler) :
    %([=](auto&&... args) { return handler(args...); })
{}

template <typename O, typename M> %::%(O* object, M method) :
    %([=](auto&&... args) { return ((*object).*(method))(args...); })
{}

template <typename O, typename M> %::%(com_ptr<O>&& object, M method) :
    %([o = std::move(object), method](auto&&... args) { return ((*o).*(method))(args...); })
{}

template <typename O, typename M> %::%(weak_ref<O>&& object, M method) :
    %([o = std::move(object), method](auto&&... args) { if (auto s = o.get()) { ((*s).*(method))(args...); } })
{}

inline % %::operator()(%) const
{%
    check_hresult((*(impl::abi_t<%>**)this)->Invoke(%));%
}
)";

        auto type_name = type.TypeName();
        auto guard{ w.push_generic_params(type.GenericParam()) };
        method_signature signature{ get_delegate_method(type) };

        w.write(format,
            type_name,
            type_name,
            type_name,
            type_name,
            type_name,
            type_name,
            type_name,
            type_name,
            type_name,
            type_name,
            type_name,
            type_name,
            type_name,
            type_name,
            type_name,
            type_name,
            signature.return_signature(),
            type_name,
            bind<write_consume_params>(signature),
            bind<write_consume_return_type>(signature),
            type_name,
            bind<write_abi_args>(signature),
            bind<write_consume_return_statement>(signature));
    }

    inline void write_struct_field(writer& w, std::pair<std::string_view, std::string> const& field)
    {
        w.write("    @ %;\n",
            field.second,
            field.first);
    }

    inline void write_struct_equality(writer& w, std::vector<std::pair<std::string_view, std::string>> const& fields)
    {
        for (size_t i = 0; i != fields.size(); ++i)
        {
            w.write(" left.% == right.%", fields[i].first, fields[i].first);

            if (i + 1 != fields.size())
            {
                w.write(" &&");
            }
        }
    }

    inline void write_structs(writer& w, std::vector<TypeDef> const& types)
    {
        auto format = R"(
struct %
{
%};

inline bool operator==(% const& left, % const& right)%
{
    return%;
}

inline bool operator!=(% const& left, % const& right)%
{
    return !(left == right);
}
)";

        if (types.empty())
        {
            return;
        }

        struct complex_struct
        {
            complex_struct(writer& w, TypeDef const& type) :
                type(type),
                is_noexcept(!has_reference(type))
            {
                for (auto&& field : type.FieldList())
                {
                    fields.emplace_back(field.Name(), w.write_temp("%", field.Signature().Type()));
                }
            }

            static bool has_reference(TypeDef const&)
            {
                return false;
            };

            TypeDef type;
            std::vector<std::pair<std::string_view, std::string>> fields;
            bool is_noexcept{ false };
        };

        std::vector<complex_struct> structs;

        for (auto&& type : types)
        {
            structs.emplace_back(w, type);
        }

        auto depends = [](writer& w, complex_struct const& left, complex_struct const& right)
        {
            for (auto&& field : left.fields)
            {
                if (w.write_temp("@::%", right.type.TypeNamespace(), right.type.TypeName()) == field.second)
                {
                    return true;
                }
            }

            return false;
        };

        for (size_t left = 0; left < structs.size(); ++left)
        {
            for (size_t right = left + 1; right < structs.size(); ++right)
            {
                if (depends(w, structs[left], structs[right]))
                {
                    // Left depends on right, therefore move right in front of left.
                    complex_struct temp = std::move(structs[right]);
                    structs.erase(structs.begin() + right);
                    structs.insert(structs.begin() + left, std::move(temp));

                    // Start over from the newly inserted struct.
                    right = structs.size();
                    --left;
                }
            }
        }

        for (auto&& type : structs)
        {
            auto name = type.type.TypeName();
            std::string_view is_noexcept = type.is_noexcept ? " noexcept" : "";

            w.write(format,
                name,
                bind_each<write_struct_field>(type.fields),
                name,
                name,
                is_noexcept,
                bind<write_struct_equality>(type.fields),
                name,
                name,
                is_noexcept);
        }
    }

    inline void write_class_requires(writer& w, TypeDef const& type)
    {
        if (type.TypeName() == "CompositionAnimationGroup")
        {
            int i = 0;
            i = i;
        }

        bool first{ true };

        for (auto&&[interface_name, info] : get_interfaces(w, type))
        {
            if (!info.defaulted || info.base)
            {
                if (first)
                {
                    first = false;
                    w.write(",\n    impl::require<%", type.TypeName());
                }

                w.write(", %", interface_name);
            }
        }

        if (!first)
        {
            w.write('>');
        }
    }

    inline void write_class_base(writer& w, TypeDef const& type)
    {
        bool first{ true };

        for (auto&& base : get_bases(type))
        {
            if (first)
            {
                first = false;
                w.write(",\n    impl::base<%", type.TypeName());
            }

            w.write(", %", base);
        }

        if (!first)
        {
            w.write('>');
        }
    }

    inline void write_constructor_declarations(writer& w, TypeDef const& type, std::vector<factory_type> const& factories)
    {
        auto type_name = type.TypeName();

        for (auto&& factory : factories)
        {
            if (factory.activatable)
            {
                if (!factory.type)
                {
                    w.write("    %();\n", type_name);
                }
                else
                {
                    for (auto&& method : factory.type.MethodList())
                    {
                        method_signature signature{ method };

                        w.write("    %(%);\n",
                            type_name,
                            bind<write_consume_params>(signature));
                    }
                }
            }
            else if (factory.composable && factory.visible)
            {
                for (auto&& method : factory.type.MethodList())
                {
                    method_signature signature{ method };
                    auto& params = signature.params();
                    params.resize(params.size() - 2);

                    w.write("    %(%);\n",
                        type_name,
                        bind<write_consume_params>(signature));
                }
            }
        }
    }

    inline void write_constructor_definition(writer& w, MethodDef const& method, std::string_view const& type_name, TypeDef const& factory)
    {
        method_signature signature{ method };

        auto format = R"(
inline %::%(%) :
    %(impl::call_factory<%, @::%>([&](auto&& f) { return f.%(%); }))
{}
)";

        w.write(format,
            type_name,
            type_name,
            bind<write_consume_params>(signature),
            type_name,
            type_name,
            factory.TypeNamespace(), factory.TypeName(),
            get_name(method),
            bind<write_consume_args>(signature));
    }

    inline void write_composable_constructor_definition(writer& w, MethodDef const& method, std::string_view const& type_name, TypeDef const& factory)
    {
        method_signature signature{ method };
        auto& params = signature.params();
        auto inner_param = params.back().first.Name();
        params.pop_back();
        auto base_param = params.back().first.Name();
        params.pop_back();

        auto format = R"(
inline %::%(%)
{
    Windows::Foundation::IInspectable %, %;
    *this = impl::call_factory<%, @::%>([&](auto&& f) { return f.%(%%%, %); });
}
)";

        w.write(format,
            type_name,
            type_name,
            bind<write_consume_params>(signature),
            base_param,
            inner_param,
            type_name,
            factory.TypeNamespace(), factory.TypeName(),
            get_name(method),
            bind<write_consume_args>(signature),
            params.empty() ? "" : ", ",
            base_param,
            inner_param);
    }


    inline void write_static_declaration(writer& w, factory_type const& factory)
    {
        if (!factory.statics)
        {
            return;
        }

        for (auto&& method : factory.type.MethodList())
        {
            method_signature signature{ method };
            auto method_name = get_name(method);
            w.async_types = is_async(method, signature);

            w.write("    static % %(%);\n",
                signature.return_signature(),
                method_name,
                bind<write_consume_params>(signature));

            if (is_add_overload(method))
            {
                auto format = R"(    using %_revoker = impl::factory_event_revoker<%, &impl::abi_t<%>::remove_%>;
    static %_revoker %(auto_revoke_t, %);
)";

                w.write(format,
                    method_name,
                    factory.type,
                    factory.type,
                    method_name,
                    method_name,
                    method_name,
                    bind<write_consume_params>(signature));
            }
        }
    }

    inline void write_static_definitions(writer& w, MethodDef const& method, std::string_view const& type_name, TypeDef const& factory)
    {
        auto format = R"(
inline % %::%(%)
{
    %impl::call_factory<%, %>([&](auto&& f) { return f.%(%); });
}
)";

        // TODO: add get_name/get_abi_name/is_async to method_signature
        method_signature signature{ method };
        auto method_name = get_name(method);
        w.async_types = is_async(method, signature);

        w.write(format,
            signature.return_signature(),
            type_name,
            method_name,
            bind<write_consume_params>(signature),
            signature.return_signature() ? "return " : "",
            type_name,
            factory,
            method_name,
            bind<write_consume_args>(signature));
    }

    inline void write_class_definitions(writer& w, TypeDef const& type)
    {
        auto type_name = type.TypeName();

        for (auto&& factory : get_factories(type))
        {
            if (factory.activatable)
            {
                if (!factory.type)
                {
                    auto format = R"(
inline %::%() :
    %(impl::call_factory<%>([](auto&& f) { return f.template ActivateInstance<%>(); }))
{}
)";

                    w.write(format,
                        type_name,
                        type_name,
                        type_name,
                        type_name,
                        type_name);
                }
                else
                {
                    w.write_each<write_constructor_definition>(factory.type.MethodList(), type_name, factory.type);
                }
            }
            else if (factory.composable && factory.visible)
            {
                w.write_each<write_composable_constructor_definition>(factory.type.MethodList(), type_name, factory.type);
            }
            else if (factory.statics)
            {
                w.write_each<write_static_definitions>(factory.type.MethodList(), type_name, factory.type);
            }
        }
    }

    inline void write_class(writer& w, TypeDef const& type)
    {
        auto guard{ w.push_generic_params(type.GenericParam()) };
        auto type_name = type.TypeName();
        auto factories = get_factories(type);

        if (auto default_interface = get_default_interface(type))
        {
            auto format = R"(
struct WINRT_EBO % :
    %%%
{
    %(std::nullptr_t) noexcept {}
%%%};
)";

            w.write(format,
                type_name,
                default_interface,
                bind<write_class_base>(type),
                bind<write_class_requires>(type),
                type_name,
                bind<write_constructor_declarations>(type, factories),
                bind<write_class_usings>(type),
                bind_each<write_static_declaration>(factories));
        }
        else
        {
            auto format = R"(
struct %
{
    %() = delete;
%};
)";

            w.write(format,
                type_name,
                type_name,
                bind_each<write_static_declaration>(factories));
        }
    }

    inline void write_std_hash(writer& w, TypeDef const& type)
    {
        w.write("template<> struct hash<winrt::%> : winrt::impl::hash_base<winrt::%> {};\n",
            type,
            type);
    }

    inline void write_namespace_special(writer& w, std::string_view const& namespace_name)
    {
        if (namespace_name == "Windows.Foundation")
        {
            w.write(strings::base_reference_produce);
        }
    }

    inline void write_base_fragment(writer& w, std::string_view const& value)
    {
        for (auto&& c : value)
        {
            if (c == '`')
            {
                w.write(settings.root);
            }
            else
            {
                w.write(c);
            }
        }
    }

    inline void write_component_include(writer& w, TypeDef const& type)
    {
        if (get_factories(type).empty())
        {
            return;
        }

        w.write("#include \"%.h\"\n", get_component_filename(type));
    }

    inline void write_component_activation(writer& w, TypeDef const& type)
    {
        if (get_factories(type).empty())
        {
            return;
        }

        auto format = R"(
        if (requal(name, L"@::%"))
        {
            *factory = winrt::detach_abi(winrt::make<winrt::@::factory_implementation::%>());
            return 0;
        }
)";

        auto type_name = type.TypeName();
        auto type_namespace = type.TypeNamespace();

        w.write(format,
            type_namespace,
            type_name,
            type_namespace,
            type_name);
    }

    inline void write_component_pch_include(writer& w)
    {
        if (settings.component_pch.empty())
        {
            return;
        }
        w.write("#include \"%\"\n", settings.component_pch);
    }

    inline void write_component_g_cpp(writer& w, std::vector<TypeDef> const& classes)
    {
        auto format = R"(%%
int32_t WINRT_CALL WINRT_CanUnloadNow() noexcept
{
#ifdef _WRL_MODULE_H_
    if (!::Microsoft::WRL::Module<::Microsoft::WRL::InProc>::GetModule().Terminate())
    {
        return 1;
    }
#endif

    if (winrt::get_module_lock())
    {
        return 1;
    }

    winrt::clear_factory_cache();
    return 0;
}

int32_t WINRT_CALL WINRT_GetActivationFactory(void* classId, void** factory) noexcept
{
    try
    {
        *factory = nullptr;
        uint32_t length{};
        wchar_t const* const buffer = WINRT_WindowsGetStringRawBuffer(classId, &length);
        std::wstring_view const name{ buffer, length };

        auto requal = [](std::wstring_view const& left, std::wstring_view const& right) noexcept
        {
            return std::equal(left.rbegin(), left.rend(), right.rbegin(), right.rend());
        };
%
#ifdef _WRL_MODULE_H_
        return ::Microsoft::WRL::Module<::Microsoft::WRL::InProc>::GetModule().GetActivationFactory(static_cast<HSTRING>(classId), reinterpret_cast<::IActivationFactory**>(factory));
#else
        return winrt::hresult_class_not_available(name).to_abi();
#endif
    }
    catch (...) { return winrt::to_hresult(); }
}
)";

        w.write(format,
            bind<write_component_pch_include>(),
            bind_each<write_component_include>(classes),
            bind_each<write_component_activation>(classes));
    }

    inline void write_component_g_h(writer& w, TypeDef const& type)
    {

    }

    inline void write_component_h(writer& w, TypeDef const& type)
    {

    }

    inline void write_component_cpp(writer& w, TypeDef const& type)
    {

    }
}
