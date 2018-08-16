#include "pch.h"
#include "version.h"
#include "strings.h"

using namespace std::chrono;
using namespace std::experimental::filesystem;
using namespace std::string_view_literals;
using namespace xlang;
using namespace xlang::meta::reader;
using namespace xlang::text;

template <typename...T> struct overloaded : T... { using T::operator()...; };
template <typename...T> overloaded(T...)->overloaded<T...>;

template <typename T>
auto get_impl_name(T const& type)
{
    std::string result;

    for (auto&& c : type.TypeNamespace())
    {
        result += c == '.' ? '_' : c;
    }

    result += '_';

    for (auto&& c : type.TypeName())
    {
        result += c == '.' ? '_' : c;
    }

    return result;
}

struct writer : writer_base<writer>
{
    using writer_base<writer>::write;

    std::string output_folder;
    std::string_view relative_folder;
    std::string type_namespace;
    bool abi_types{};
    bool param_names{};
    bool consume_types{};
    bool async_types{};
    std::map<std::string_view, std::set<TypeDef>> depends;

    void add_depends(TypeDef const& type)
    {
        XLANG_ASSERT(!type_namespace.empty());
        auto ns = type.TypeNamespace();

        if (ns != type_namespace)
        {
            depends[ns].insert(type);
        }
    }

    std::vector<std::pair<GenericParam, GenericParam>> generic_param_stack;

    struct generic_param_guard
    {
        explicit generic_param_guard(writer* arg)
            : owner(arg)
        {}

        ~generic_param_guard()
        {
            if (owner)
            {
                owner->generic_param_stack.pop_back();
            }
        }

        generic_param_guard(generic_param_guard && other)
            : owner(other.owner)
        {
            owner = nullptr;
        }
        generic_param_guard& operator=(generic_param_guard const&) = delete;
        writer* owner;
    };

    generic_param_guard push_generic_params(std::pair<GenericParam, GenericParam>&& arg)
    {
        if (!empty(arg))
        {
            generic_param_stack.push_back(std::move(arg));
            return generic_param_guard{ this };
        }

        return generic_param_guard{ nullptr };
    }

    void write_value(bool value)
    {
        write(value ? "TRUE"sv : "FALSE"sv);
    }

    void write_value(char16_t value)
    {
        write_printf("%#0hx", value);
    }

    void write_value(int8_t value)
    {
        write_printf("%hhd", value);
    }

    void write_value(uint8_t value)
    {
        write_printf("%#0hhx", value);
    }

    void write_value(int16_t value)
    {
        write_printf("%hd", value);
    }

    void write_value(uint16_t value)
    {
        write_printf("%#0hx", value);
    }

    void write_value(int32_t value)
    {
        write_printf("%d", value);
    }

    void write_value(uint32_t value)
    {
        write_printf("%#0x", value);
    }

    void write_value(int64_t value)
    {
        write_printf("%lld", value);
    }

    void write_value(uint64_t value)
    {
        write_printf("%#0llx", value);
    }

    void write_value(float value)
    {
        write_printf("%f", value);
    }

    void write_value(double value)
    {
        write_printf("%f", value);
    }

    void write_value(std::string_view value)
    {
        write("\"%\"", value);
    }

    void write_code(std::string_view const& value)
    {
        for (auto&& c : value)
        {
            if (c == '.')
            {
                write("::");
            }
            else if (c == '`')
            {
                return;
            }
            else
            {
                write(c);
            }
        }
    }

    void write(Constant const& value)
    {
        switch (value.Type())
        {
        case ConstantType::Boolean:
            write_value(value.ValueBoolean());
            break;
        case ConstantType::Char:
            write_value(value.ValueChar());
            break;
        case ConstantType::Int8:
            write_value(value.ValueInt8());
            break;
        case ConstantType::UInt8:
            write_value(value.ValueUInt8());
            break;
        case ConstantType::Int16:
            write_value(value.ValueInt16());
            break;
        case ConstantType::UInt16:
            write_value(value.ValueUInt16());
            break;
        case ConstantType::Int32:
            write_value(value.ValueInt32());
            break;
        case ConstantType::UInt32:
            write_value(value.ValueUInt32());
            break;
        case ConstantType::Int64:
            write_value(value.ValueInt64());
            break;
        case ConstantType::UInt64:
            write_value(value.ValueUInt64());
            break;
        case ConstantType::Float32:
            write_value(value.ValueFloat32());
            break;
        case ConstantType::Float64:
            write_value(value.ValueFloat64());
            break;
        case ConstantType::String:
            write_value(value.ValueString());
            break;
        case ConstantType::Class:
            write("null");
            break;
        }
    }

    //void write_struct_abi(std::string_view const& ns, std::string_view const& name)
    //{
    //    write("struct struct_%")
    //}

    void write(TypeDef const& type)
    {
        auto ns = type.TypeNamespace();
        auto name = type.TypeName();
        add_depends(type);

        if (abi_types)
        {
            auto category = get_category(type);

            if (name == "EventRegistrationToken" && ns == "Windows.Foundation")
            {
                write("int64_t");
            }
            else if (category == category::struct_type)
            {
                write("struct struct_%", get_impl_name(type));
            }
            else if (category == category::enum_type)
            {
                write(type.FieldList().first.Signature().Type());
            }
            else
            {
                write("void*");
            }
        }
        else
        {
            if (name == "EventRegistrationToken" && ns == "Windows.Foundation")
            {
                write("winrt::event_token");
            }
            else
            {
                write("@::%", ns, name);
            }
        }
    }

    void write(TypeRef const& type)
    {
        auto ns = type.TypeNamespace();
        auto name = type.TypeName();

        if (name == "Guid" && ns == "System")
        {
            write("winrt::guid");
        }
        else
        {
            write(find(type));
        }
    }

    void write(TypeSpec const& type)
    {
        write(type.Signature().GenericTypeInst());
    }

    void write(coded_index<TypeDefOrRef> const& type)
    {
        switch (type.type())
        {
        case TypeDefOrRef::TypeDef:

            write(type.TypeDef());
            break;

        case TypeDefOrRef::TypeRef:
            write(type.TypeRef());
            break;

        case TypeDefOrRef::TypeSpec:
            write(type.TypeSpec());
            break;
        }
    }

    void write(GenericTypeInstSig const& type)
    {
        if (abi_types)
        {
            write("void*");
        }
        else
        {
            auto generic_type = type.GenericType().TypeRef();
            auto ns = generic_type.TypeNamespace();
            auto name = generic_type.TypeName();
            name.remove_suffix(name.size() - name.rfind('`'));

            if (consume_types)
            {
                //debug_trace = true;

                static constexpr std::string_view optional("Windows::Foundation::IReference"sv);
                static constexpr std::string_view iterable("Windows::Foundation::Collections::IIterable"sv);
                static constexpr std::string_view vector_view("Windows::Foundation::Collections::IVectorView"sv);
                static constexpr std::string_view map_view("Windows::Foundation::Collections::IMapView"sv);
                static constexpr std::string_view vector("Windows::Foundation::Collections::IVector"sv);
                static constexpr std::string_view map("Windows::Foundation::Collections::IMap"sv);

                consume_types = false;
                auto full_name = write_temp("@::%<%>", ns, name, bind_list(", ", type.GenericArgs()));
                consume_types = true;

                if (starts_with(full_name, optional))
                {
                    write("optional%", full_name.substr(optional.size()));
                }
                else if (starts_with(full_name, iterable))
                {
                    if (async_types)
                    {
                        write("param::async_iterable%", full_name.substr(iterable.size()));
                    }
                    else
                    {
                        write("param::iterable%", full_name.substr(iterable.size()));
                    }
                }
                else if (starts_with(full_name, vector_view))
                {
                    if (async_types)
                    {
                        write("param::async_vector_view%", full_name.substr(vector_view.size()));
                    }
                    else
                    {
                        write("param::vector_view%", full_name.substr(vector_view.size()));
                    }
                }

                else if (starts_with(full_name, map_view))
                {
                    if (async_types)
                    {
                        write("param::async_map_view%", full_name.substr(map_view.size()));
                    }
                    else
                    {
                        write("param::map_view%", full_name.substr(map_view.size()));
                    }
                }
                else if (starts_with(full_name, vector))
                {
                    write("param::vector%", full_name.substr(vector.size()));
                }
                else if (starts_with(full_name, map))
                {
                    write("param::map%", full_name.substr(map.size()));
                }
                else
                {
                    write(full_name);
                }
            }
            else
            {
                write("@::%<%>", ns, name, bind_list(", ", type.GenericArgs()));
            }
        }
    }

    void write(TypeSig const& signature)
    {
        std::visit(overloaded
            {
                [&](ElementType type)
                {
                    if (type == ElementType::Boolean) { write("bool");   }
                    else if (type == ElementType::Char) { write("char16_t");  }
                    else if (type == ElementType::I1) { write("int8_t");   }
                    else if (type == ElementType::U1) { write("uint8_t");  }
                    else if (type == ElementType::I2) { write("int16_t");  }
                    else if (type == ElementType::U2) { write("uint16_t"); }
                    else if (type == ElementType::I4) { write("int32_t");  }
                    else if (type == ElementType::U4) { write("uint32_t"); }
                    else if (type == ElementType::I8) { write("int64_t");  }
                    else if (type == ElementType::U8) { write("uint64_t"); }
                    else if (type == ElementType::R4) { write("float");    }
                    else if (type == ElementType::R8) { write("double");   }
                    else if (type == ElementType::String)
                    {
                        if (abi_types)
                        {
                            write("void*");
                        }
                        else if (consume_types)
                        {
                            write("param::hstring");
                        }
                        else
                        {
                            write("hstring");
                        }
                    }
                    else if (type == ElementType::Object)
                    {
                        if (abi_types)
                        {
                            write("void*");
                        }
                        else
                        {
                            write("Windows::Foundation::IInspectable");
                        }
                    }
                    else
                    {
                        XLANG_ASSERT(false);
                    }
                },
                [&](GenericTypeIndex var)
                {
                    write("%", begin(generic_param_stack.back())[var.index].Name());
                },
                [&](auto&& type)
                {
                    write(type);
                }
            },
            signature.Type());
    }

    void write(InterfaceImpl const& impl)
    {
        write(impl.Interface());
    }

    void write(FixedArgSig const& arg)
    {
        std::visit(overloaded{
            [this](ElemSig::SystemType arg)
        {
            write(arg.name);
        },
            [this](ElemSig::EnumValue arg)
        {
            // TODO: Map the integer value to an enumerator
            std::visit([this](auto&& value) { write_value(value); }, arg.value);
        },
            [this](auto&& arg)
        {
            write_value(arg);
        }
            }, std::get<ElemSig>(arg.value).value);
    }

    void write(NamedArgSig const& arg)
    {
        write(arg.value);
    }

    void write(RetTypeSig const& value)
    {
        if (value)
        {
            write(value.Type());
        }
        else
        {
            write("void");
        }
    }

    void write_license()
    {
        write("// WARNING: Please don't edit this file. It was generated by C++/WinRT v%\n", XLANG_VERSION_STRING);
    }

    void write_include_guard()
    {
        write("\n#pragma once\n");
    }

    void write_depends(std::string_view const& ns, char impl = 0)
    {
        XLANG_ASSERT(!relative_folder.empty());

        if (impl)
        {
            write("#include \"%/impl/%.%.h\"\n", relative_folder, ns, impl);
        }
        else
        {
            write("#include \"%/%.h\"\n", relative_folder, ns);
        }
    }

    void save_header(char impl = 0)
    {
        auto filename{ output_folder };

        if (impl)
        {
            filename += "impl";
            filename += static_cast<char>(path::preferred_separator);
        }

        filename += type_namespace;

        if (impl)
        {
            filename += '.';
            filename += impl;
        }

        filename += ".h";
        flush_to_file(filename);
    }
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

struct method_signature
{
    explicit method_signature(MethodDef const& method) :
        m_method(method.Signature()),
        m_params(method.ParamList())
    {
        if (m_method.ReturnType() && m_params.first != m_params.second && m_params.first.Sequence() == 0)
        {
            m_return = m_params.first;
            ++m_params.first;
        }

        XLANG_ASSERT(meta::reader::size(m_params) == m_method.Params().size());
    }

    generator<std::pair<Param const&, ParamSig const&>> params() const noexcept
    {
        for (uint32_t i{}; i != m_method.Params().size(); ++i)
        {
            co_yield{ m_params.first + i, m_method.Params()[i] };
        }
    }

    auto const& return_signature() const noexcept
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
            name = "generic_return_param_name";
        }

        return name;
    }

    bool has_params() const noexcept
    {
        return m_method.Params().size();
    }

private:

    MethodDefSig m_method;
    std::pair<Param, Param> m_params;
    Param m_return;
};

InterfaceImpl get_default_interface(TypeDef const& type)
{
    auto impls = type.InterfaceImpl();

    for (auto&& impl : impls)
    {
        if (get_attribute(impl, "Windows.Foundation.Metadata", "DefaultAttribute"))
        {
            return impl;
        }
    }

    if (impls.first != impls.second)
    {
        return impls.first;
    }

    return {};
}

bool is_in(Param const& param)
{
    return enum_mask(param.Flags(), ParamAttributes::In) == ParamAttributes::In;
}

bool is_out(Param const& param)
{
    return enum_mask(param.Flags(), ParamAttributes::Out) == ParamAttributes::Out;
}

auto get_abi_name(MethodDef const& method)
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

auto get_name(MethodDef const& method)
{
    auto name = method.Name();

    if (enum_mask(method.Flags(), MethodAttributes::SpecialName) == MethodAttributes::SpecialName)
    {
        return name.substr(name.find('_') + 1);
    }

    return name;
}

bool has_special_name(MethodDef const& method)
{
    return enum_mask(method.Flags(), MethodAttributes::SpecialName) == MethodAttributes::SpecialName;
}

bool is_remove_overload(MethodDef const& method)
{
    return has_special_name(method) && starts_with(method.Name(), "remove_");
}

bool is_add_overload(MethodDef const& method)
{
    return has_special_name(method) && starts_with(method.Name(), "add_");
}

bool is_noexcept(MethodDef const& method)
{
    return is_remove_overload(method) || get_attribute(method, "Experimental.Windows.Foundation.Metadata", "NoExceptAttribute");
}

void write_impl_namespace(writer& w)
{
    w.write("\nnamespace winrt::impl {\n");
}

void write_std_namespace(writer& w)
{
    w.write("\nWINRT_EXPORT namespace std {\n");
}

void write_type_namespace(writer& w, std::string_view const& ns)
{
    w.write("\nWINRT_EXPORT namespace winrt::@ {\n", ns);
}

void write_close_namespace(writer& w)
{
    w.write("\n}\n");
}

void write_enum_field(writer& w, Field const& field)
{
    if (auto const& constant = field.Constant())
    {
        w.write("\n    % = %,",
            field.Name(),
            *constant);
    }
}

void write_enum(writer& w, TypeDef const& type)
{
    auto fields = type.FieldList();

    w.write(
R"(
enum class % : %
{%
};
)",
        type.TypeName(),
        fields.first.Signature().Type(),
        bind_each<write_enum_field>(fields));
}

void write_forward(writer& w, TypeDef const& type)
{
    auto category = get_category(type);

    if (category == category::enum_type)
    {
        w.write("enum class % : %;\n", type.TypeName(), type.FieldList().first.Signature().Type());
    }
    else
    {
        w.write("struct %;\n", type.TypeName());
    }
}

void write_enum_flag(writer& w, TypeDef const& type)
{
    if (get_attribute(type, "System", "FlagsAttribute"))
    {
        w.write("template<> struct is_enum_flag<@::%> : std::true_type {};\n",
            type.TypeNamespace(),
            type.TypeName());
    }
}

void write_category(writer& w, TypeDef const& type, std::string_view const& category)
{
    w.write("template <> struct category<%>{ using type = %; };\n",
        type,
        category);
}

void write_name(writer& w, TypeDef const& type)
{
    auto ns = type.TypeNamespace();
    auto name = type.TypeName();

    w.write("template <> struct name<@::%>{ static constexpr auto & value{ L\"%.%\" }; };\n",
        ns, name, ns, name);
}

void write_guid_value(writer& w, std::vector<FixedArgSig> const& args)
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

void write_guid(writer& w, TypeDef const& type)
{
    auto attribute = get_attribute(type, "Windows.Foundation.Metadata", "GuidAttribute");

    w.write("template <> struct guid_storage<%>{ static constexpr guid value{ % }; };\n",
        type,
        bind<write_guid_value>(attribute.Value().FixedArgs()));
}

void write_default_interface(writer& w, TypeDef const& type)
{
    if (auto default_interface = get_default_interface(type))
    {
        w.write("template <> struct default_interface<%>{ using type = %; };\n",
            type,
            default_interface);
    }
}

void write_field_types(writer& w, TypeDef const& type)
{
    separator s{ w };

    for (auto&& field : type.FieldList())
    {
        s();

        w.write(field.Signature().Type());
    }
}

void write_struct_category(writer& w, TypeDef const& type)
{
    w.write("template <> struct category<%>{ using type = struct_category<%>; };\n",
        type,
        bind<write_field_types>(type));
}

void write_array_size_name(writer& w, Param const& param)
{
    if (w.param_names)
    {
        w.write(" __%Size", param.Name());
    }
}

void write_abi_params(writer& w, method_signature const& method_signature)
{
    separator s{ w };

    for (auto&&[param, param_signature] : method_signature.params())
    {
        s();

        if (param_signature.Type().is_szarray())
        {
            std::string_view format;

            if (is_in(param))
            {
                format = "uint32_t%, %*";
            }
            else if (param_signature.ByRef())
            {
                format = "uint32_t*%, %**";
            }
            else
            {
                format = "uint32_t%, %*";
            }

            w.write(format, bind<write_array_size_name>(param), param_signature.Type());
        }
        else
        {
            w.write(param_signature.Type());

            if (is_in(param))
            {
                XLANG_ASSERT(!is_out(param));

                if (is_const(param_signature))
                {
                    w.write(" const&");
                }
            }
            else
            {
                XLANG_ASSERT(!is_in(param));
                XLANG_ASSERT(is_out(param));

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
            w.write("uint32_t, %*", type);
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

bool wrap_abi(TypeSig const& signature)
{
    bool wrap{};

    std::visit(overloaded
        {
            [&](ElementType type)
            {
                wrap = type == ElementType::String || type == ElementType::Object;
            },
            [&](auto&&)
            {
                wrap = true;
            }
        },
        signature.Type());

    return wrap;
}

void write_abi_args(writer& w, method_signature const& method_signature)
{
    separator s{ w };

    for (auto&&[param, param_signature] : method_signature.params())
    {
        s();
        auto param_name = param.Name();

        if (param_signature.Type().is_szarray())
        {
            std::string_view format;

            if (is_in(param))
            {
                format = "%.size(), get_abi(%)";
            }
            else if (param_signature.ByRef())
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
            if (is_in(param))
            {
                XLANG_ASSERT(!is_out(param));

                if (wrap_abi(param_signature.Type()))
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
                XLANG_ASSERT(!is_in(param));
                XLANG_ASSERT(is_out(param));

                if (wrap_abi(param_signature.Type()))
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
            std::string_view format;

            if (method_signature.return_signature().ByRef())
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

void write_abi_declaration(writer& w, MethodDef const& method)
{
    method_signature signature{ method };

    w.write("    virtual int32_t WINRT_CALL %(%) noexcept = 0;\n",
        get_abi_name(method),
        bind<write_abi_params>(signature));
}

void write_interface_abi(writer& w, TypeDef const& type)
{
    auto guard{ w.push_generic_params(type.GenericParam()) };

    w.write(
R"(
template <> struct abi<@::%>{ struct type : IInspectable
{
%};};
)",
        type.TypeNamespace(),
        type.TypeName(),
        bind_each<write_abi_declaration>(type.MethodList()));
}

auto get_delegate_method(TypeDef const& type)
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

void write_delegate_abi(writer& w, TypeDef const& type)
{
    auto guard{ w.push_generic_params(type.GenericParam()) };

    w.write(
R"(
template <> struct abi<@::%>{ struct type : IUnknown
{
%};};
)",
        type.TypeNamespace(),
        type.TypeName(),
        bind<write_abi_declaration>(get_delegate_method(type)));
}

std::string get_field_abi(writer& w, Field const& field)
{
    auto signature = field.Signature();
    auto const& type = signature.Type();
    std::string name = w.write_temp("%", type);

    if (starts_with(name, "struct "))
    {
        auto ref = std::get<coded_index<TypeDefOrRef>>(type.Type());
        XLANG_ASSERT(ref.type() == TypeDefOrRef::TypeRef);

        name = "struct{";

        for (auto&& nested : find(ref.TypeRef()).FieldList())
        {
            name += " " + get_field_abi(w, nested) + " ";
            name += nested.Name();
            name += ";";
        }

        name += " }";
    }

    return name;
}

void write_field_abi(writer& w, Field const& field)
{
    w.write("    % %;\n", get_field_abi(w, field), field.Name());
}

void write_struct_abi(writer& w, TypeDef const& type)
{
    auto impl_name = get_impl_name(type);
    
    w.write(
R"(
struct struct_%
{
%};
template <> struct abi<@::%>{ using type = struct_%; };
)",
        impl_name,
        bind_each<write_field_abi>(type.FieldList()),
        type.TypeNamespace(),
        type.TypeName(),
        impl_name);

}

void write_consume_params(writer& w, method_signature const& method_signature)
{
    separator s{ w };

    for (auto&&[param, param_signature] : method_signature.params())
    {
        s();

        if (param_signature.Type().is_szarray())
        {
            std::string_view format;

            if (is_in(param))
            {
                format = "array_view<% const>";
            }
            else if (param_signature.ByRef())
            {
                format = "com_array<%>&";
            }
            else
            {
                format = "array_view<%>";
            }

            w.write(format, param_signature.Type());
        }
        else
        {
            if (is_in(param))
            {
                XLANG_ASSERT(!is_out(param));
                w.consume_types = true;

                auto param_type = std::get_if<ElementType>(&param_signature.Type().Type());

                if (param_type && *param_type != ElementType::String && *param_type != ElementType::Object)
                {
                    w.write("%", param_signature.Type());
                }
                else
                {
                    w.write("% const&", param_signature.Type());
                }

                w.consume_types = false;
            }
            else
            {
                XLANG_ASSERT(!is_in(param));
                XLANG_ASSERT(is_out(param));

                w.write("%&", param_signature.Type());
            }
        }

        w.write(" %", param.Name());
    }
}

void write_implementation_params(writer& w, method_signature const& method_signature)
{
    separator s{ w };

    for (auto&&[param, param_signature] : method_signature.params())
    {
        s();

        if (param_signature.Type().is_szarray())
        {
            std::string_view format;

            if (is_in(param))
            {
                format = "array_view<% const>";
            }
            else if (param_signature.ByRef())
            {
                format = "com_array<%>&";
            }
            else
            {
                format = "array_view<%>";
            }

            w.write(format, param_signature.Type());
        }
        else
        {
            if (is_in(param))
            {
                XLANG_ASSERT(!is_out(param));

                auto param_type = std::get_if<ElementType>(&param_signature.Type().Type());

                if (param_type && *param_type != ElementType::String && *param_type != ElementType::Object)
                {
                    w.write("%", param_signature.Type());
                }
                else
                {
                    w.write("% const&", param_signature.Type());
                }
            }
            else
            {
                XLANG_ASSERT(!is_in(param));
                XLANG_ASSERT(is_out(param));

                w.write("%&", param_signature.Type());
            }
        }

        if (w.param_names)
        {
            w.write(" %", param.Name());
        }
    }
}

void write_consume_declaration(writer& w, MethodDef const& method, TypeDef const& type)
{
    method_signature signature{ method };
    auto method_name = get_name(method);

    w.write("    % %(%) const%;\n",
        signature.return_signature(),
        method_name,
        bind<write_consume_params>(signature),
        is_noexcept(method) ? " noexcept" : "");

    if (is_add_overload(method))
    {
        w.write(
R"(    using %_revoker = impl::event_revoker<%, &impl::abi_t<%>::remove_%>;
    %_revoker %(auto_revoke_t, %) const;
)",
            method_name,
            type,
            type,
            method_name,
            method_name,
            method_name,
            bind<write_consume_params>(signature));
    }
}

void write_consume_return_type(writer& w, method_signature const& signature)
{
    if (signature.return_signature())
    {
        std::string_view format;

        auto return_type = std::get_if<coded_index<TypeDefOrRef>>(&signature.return_signature().Type().Type());
        bool is_class{ false };

        if (return_type)
        {
            XLANG_ASSERT(return_type->type() == TypeDefOrRef::TypeRef);

            auto ns = return_type->TypeRef().TypeNamespace();
            auto name = return_type->TypeRef().TypeName();

            if (auto definition = find(return_type->TypeRef()))
            {
                is_class = get_category(definition) == category::class_type;
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

void write_consume_return_statement(writer& w, method_signature const& signature)
{
    if (signature.return_signature())
    {
        w.write("\n    return %;", signature.return_param_name());
    }
}

void write_consume_args(writer& w, method_signature const& method_signature)
{
    separator s{ w };

    for (auto&&[param, param_signature] : method_signature.params())
    {
        s();
        w.write(param.Name());
    }
}

void write_consume_definition(writer& w, MethodDef const& method, TypeDef const& type)
{
    auto method_name = get_name(method);
    method_signature signature{ method };
    auto type_impl_name = get_impl_name(type);

    // TODO: something wrong with Parent that it doesn't always return the correct TypeDef.
    //XLANG_ASSERT(type == method.Parent());

    std::string_view format;

    if (is_noexcept(method))
    {
        format =
R"(
template <typename D> % consume_%<D>::%(%) const noexcept
{%
    WINRT_VERIFY_(0, WINRT_SHIM(%)->%(%));%
}
)";
    }
    else
    {
        format =
            R"(
template <typename D> % consume_%<D>::%(%) const
{%
    check_hresult(WINRT_SHIM(%)->%(%));%
}
)";
    }

    w.write(format,
        signature.return_signature(),
        type_impl_name,
        method_name,
        bind<write_consume_params>(signature),
        bind<write_consume_return_type>(signature),
        type,
        get_abi_name(method),
        bind<write_abi_args>(signature),
        bind<write_consume_return_statement>(signature));

    if (is_add_overload(method))
    {
        w.write(
R"(
template <typename D> typename consume_%<D>::%_revoker consume_%<D>::%(auto_revoke_t, %) const
{
    return impl::make_event_revoker<D, %_revoker>(this, %(%));
}
)",
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

void write_consume_definitions(writer& w, TypeDef const& type)
{
    auto guard{ w.push_generic_params(type.GenericParam()) };

    w.write_each<write_consume_definition>(type.MethodList(), type);
}

void write_consume(writer& w, TypeDef const& type)
{
    auto guard{ w.push_generic_params(type.GenericParam()) };
    auto impl_name = get_impl_name(type);

    w.write(
R"(
template <typename D>
struct consume_%
{
%%};
template <> struct consume<%> { template <typename D> using type = consume_%<D>; };
)",
        impl_name,
        bind_each<write_consume_declaration>(type.MethodList(), type),
        "", // extensions?
        type,
        impl_name);
}

void write_produce_params(writer& w, method_signature const& signature)
{
    w.abi_types = true;
    w.param_names = true;
    write_abi_params(w, signature);
    w.abi_types = false;
    w.param_names = false;
}

void write_produce_cleanup_param(writer& w, TypeSig const& signature, std::string_view const& param_name)
{
    if (signature.is_szarray())
    {
        w.write(
            R"(
            *__%Size = 0;
            *% = nullptr;)",
            param_name,
            param_name);

        return;
    }

    bool clear{};

    std::visit(overloaded
        {
            [&](ElementType type)
            {
                clear = type == ElementType::String || type == ElementType::Object;
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
            [&](auto&&)
            {
                clear = true;
            }
        },
        signature.Type());

    if (clear)
    {
        w.write("\n            *% = nullptr;", param_name);
    }
}

void write_produce_cleanup(writer& w, method_signature const& method_signature)
{
    for (auto&&[param, param_signature] : method_signature.params())
    {
        if (is_in(param))
        {
            continue;
        }

        write_produce_cleanup_param(w, param_signature.Type(), param.Name());
    }

    if (method_signature.return_signature())
    {
        write_produce_cleanup_param(w, method_signature.return_signature().Type(), method_signature.return_param_name());
    }
}

void write_produce_optional_definitions(writer&, method_signature const& signature)
{

}

void write_produce_assert_params(writer& w, method_signature const& method_signature)
{
    if (method_signature.has_params())
    {
        w.write(", ");
        write_implementation_params(w, method_signature);
    }
}

void write_produce_assert(writer& w, MethodDef const& method, method_signature const& signature)
{
    w.write("WINRT_ASSERT_DECLARATION(%, WINRT_WRAP(%)%);",
        get_name(method),
        signature.return_signature(),
        bind<write_produce_assert_params>(signature));
}

void write_produce_args(writer& w, method_signature const& method_signature)
{
    separator s{ w };

    for (auto&&[param, param_signature] : method_signature.params())
    {
        s();
        auto param_name = param.Name();
        auto param_type = w.write_temp("%", param_signature.Type());

        if (param_signature.Type().is_szarray())
        {
            if (is_in(param))
            {
                w.write("array_view<@ const>(reinterpret_cast<@ const *>(%), reinterpret_cast<@ const *>(%) + __%Size)",
                    param_type,
                    param_type,
                    param_name,
                    param_type,
                    param_name,
                    param_name);
            }
            else if (param_signature.ByRef())
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
            if (is_in(param))
            {
                XLANG_ASSERT(!is_out(param));

                if (wrap_abi(param_signature.Type()))
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
            // TODO: else if optional out
            else
            {
                XLANG_ASSERT(!is_in(param));
                XLANG_ASSERT(is_out(param));

                if (wrap_abi(param_signature.Type()))
                {
                    w.write("*reinterpret_cast<@*>(%)",
                        param_type,
                        param_name);
                }
                else
                {
                    w.write(param_name);
                }
            }
        }
    }
}

void write_produce_upcall(writer& w, MethodDef const& method, method_signature const& method_signature)
{
    if (method_signature.return_signature())
    {
        auto name = method_signature.return_param_name();

        if (method_signature.return_signature().Type().is_szarray())
        {
            w.write("std::tie(*__%Size, *%) = detach_abi(this->shim().%(%))",
                name,
                name,
                get_name(method),
                bind<write_produce_args>(method_signature));
        }
        else
        {
            w.write("*% = detach_from<%>(this->shim().%(%))",
                name,
                method_signature.return_signature(),
                get_name(method),
                bind<write_produce_args>(method_signature));
        }
    }
    else
    {
        w.write("this->shim().%(%)",
            get_name(method),
            bind<write_produce_args>(method_signature));
    }
}

void write_delegate_upcall(writer& w, method_signature const& method_signature)
{
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

void write_produce_optional_results(writer&, method_signature const& signature)
{

}

void write_produce_method(writer& w, MethodDef const& method)
{
    std::string_view format;

    if (is_noexcept(method))
    {
        format =
R"(
    int32_t WINRT_CALL %(%) noexcept final
    {%
        typename D::abi_guard guard(this->shim());%
        %
        %;%
        return 0;
    }
)";
    }
    else
    {
        format =
R"(
    int32_t WINRT_CALL %(%) noexcept final
    {
        try
        {%
            typename D::abi_guard guard(this->shim());%
            %
            %;%
            return 0;
        }
        catch (...) { return to_hresult(); }
    }
)";
    }

    method_signature signature{ method };

    w.write(format,
        get_abi_name(method),
        bind<write_produce_params>(signature),
        bind<write_produce_cleanup>(signature),
        bind<write_produce_optional_definitions>(signature),
        bind<write_produce_assert>(method, signature),
        bind<write_produce_upcall>(method, signature),
        bind<write_produce_optional_results>(signature));
}

void write_produce(writer& w, TypeDef const& type)
{
    auto guard{ w.push_generic_params(type.GenericParam()) };

    w.write(
R"(
template <typename D>
struct produce<D, %> : produce_base<D, %>
{%};
)",
        type,
        type,
        bind_each<write_produce_method>(type.MethodList()));
}

void append_requires(writer& w, std::set<std::string>& names, std::pair<InterfaceImpl, InterfaceImpl> const& requires)
{
    for (auto&& type : requires)
    {
        auto base = type.Interface();
        names.insert(w.write_temp("%", base));

        switch (base.type())
        {
        case TypeDefOrRef::TypeDef:
        {
            append_requires(w, names, base.TypeDef().InterfaceImpl());
            break;
        }
        case TypeDefOrRef::TypeRef:
        {
            auto definition = find(base.TypeRef());

            if (!definition)
            {
                throw_invalid("Required interface '", base.TypeRef().TypeNamespace(), ".", base.TypeRef().TypeName(), "' could not be resolved");
            }

            w.add_depends(definition);
            append_requires(w, names, definition.InterfaceImpl());
            break;
        }
        case TypeDefOrRef::TypeSpec:
        {
            //auto signature = base.TypeSpec().Signature().GenericTypeInst();
            //XLANG_ASSERT(signature.GenericType().type() == TypeDefOrRef::TypeRef);
            //auto definition = find(signature.GenericType().TypeRef());
            //append_requires(w, names, definition.InterfaceImpl());
            break;
        }
        }
    }
}

void write_interface_requires(writer& w, TypeDef const& type)
{
    auto interfaces = type.InterfaceImpl();

    if (empty(interfaces))
    {
        return;
    }

    std::set<std::string> names;
    append_requires(w, names, interfaces);

    w.write(",\n    impl::require<%", type.TypeName());

    for (auto&& name : names)
    {
        w.write(", %", name);
    }

    w.write('>');
}

void write_interface(writer& w, TypeDef const& type)
{
    auto type_name = type.TypeName();

    auto guard{ w.push_generic_params(type.GenericParam()) };

    w.write(
R"(
struct WINRT_EBO % :
    Windows::Foundation::IInspectable,
    impl::consume_t<%>%
{
    %(std::nullptr_t = nullptr) noexcept {}
%};
)",
        type_name,
        type_name,
        bind<write_interface_requires>(type),
        type_name,
        ""); // usings...
}

void write_delegate(writer& w, TypeDef const& type)
{
    auto type_name = type.TypeName();
    auto guard{ w.push_generic_params(type.GenericParam()) };
    method_signature signature{ get_delegate_method(type) };

    w.write(
R"(
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
)",
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

void write_delegate_implementation(writer& w, TypeDef const& type)
{
    auto guard{ w.push_generic_params(type.GenericParam()) };
    auto type_name = type.TypeName();
    auto type_namespace = type.TypeNamespace();
    method_signature signature{ get_delegate_method(type) };

    w.write(
R"(
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
)",
        type_namespace, type_name,
        type_namespace, type_name,
        type_namespace, type_name,
        bind<write_abi_params>(signature),
        bind<write_delegate_upcall>(signature),
        "");
}

bool has_reference(TypeDef const& type)
{
    return false;
}

void write_struct_field(writer& w, std::pair<std::string_view, std::string> const& field)
{
    w.write("    @ %;\n",
        field.second,
        field.first);
}

void write_struct_equality(writer& w, std::vector<std::pair<std::string_view, std::string>> const& fields)
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

void write_structs(writer& w, std::vector<TypeDef> const& types)
{
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

        w.write(
R"(
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
)",
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

void write_class_base(writer& w, TypeDef const& type)
{

}

void write_class_requires(writer& w, TypeDef const& type)
{

}

TypeDef get_system_type(CustomAttribute const& attribute)
{
    auto signature = attribute.Value();

    for (auto&& arg : signature.FixedArgs())
    {
        if (auto type_param = std::get_if<ElemSig::SystemType>(&std::get<ElemSig>(arg.value).value))
        {
            return attribute.get_cache().find(type_param->name);
        }
    }

    return {};
}

auto get_activatable_factories(TypeDef const& type)
{
    std::vector<TypeDef> factories;

    for (auto&& attribute : type.CustomAttribute())
    {
        auto attribute_name = attribute.TypeNamespaceAndName();

        if (attribute_name.second == "ActivatableAttribute" && attribute_name.first == "Windows.Foundation.Metadata")
        {
            factories.push_back(get_system_type(attribute));
        }
    }

    return factories;
}

auto get_static_factories(TypeDef const& type)
{
    std::vector<TypeDef> factories;

    for (auto&& attribute : type.CustomAttribute())
    {
        auto attribute_name = attribute.TypeNamespaceAndName();

        if (attribute_name.second == "StaticAttribute" && attribute_name.first == "Windows.Foundation.Metadata")
        {
            factories.push_back(get_system_type(attribute));
        }
    }

    return factories;
}

void write_constructor_declaration(writer& w, MethodDef const& method, std::string_view const& type_name)
{
    method_signature signature{ method };

    w.write("    %(%);\n",
        type_name,
        bind<write_consume_params>(signature));
}

void write_constructor_declarations(writer& w, TypeDef const& type)
{
    auto type_name = type.TypeName();

    for (auto&& factory : get_activatable_factories(type))
    {
        if (!factory)
        {
            w.write("    %();\n", type_name);
        }
        else
        {
            w.write_each<write_constructor_declaration>(factory.MethodList(), type_name);
        }
    }
}

void write_constructor_definition(writer& w, MethodDef const& method, std::string_view const& type_name, TypeDef const& factory)
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

void write_constructor_definitions(writer& w, TypeDef const& type)
{
    auto type_name = type.TypeName();

    for (auto&& factory : get_activatable_factories(type))
    {
        if (!factory)
        {
            w.write(R"(
inline %::%() :
    %(impl::call_factory<%>([](auto&& f) { return f.template ActivateInstance<%>(); }))
{}
)",
                type_name,
                type_name,
                type_name,
                type_name,
                type_name);
        }
        else
        {
            w.write_each<write_constructor_definition>(factory.MethodList(), type_name, factory);
        }
    }
}

void write_static_declaration(writer& w, MethodDef const& method)
{
    method_signature signature{ method };

    w.write("    static % %(%);\n",
        signature.return_signature(),
        get_name(method),
        bind<write_consume_params>(signature));
}

void write_static_declarations(writer& w, TypeDef const& type)
{
    for (auto&& factory : get_static_factories(type))
    {
        w.write_each<write_static_declaration>(factory.MethodList());
    }
}

void write_static_definition(writer& w, MethodDef const& method, std::string_view const& type_name, TypeDef const& factory)
{
    method_signature signature{ method };
    auto method_name = get_name(method);

    auto format = R"(
inline % %::%(%)
{
    %impl::call_factory<%, @::%>([&](auto&& f) { return f.%(%); });
}
)";

    w.write(format,
        signature.return_signature(),
        type_name,
        method_name,
        bind<write_consume_params>(signature),
        signature.return_signature() ? "return " : "",
        type_name,
        factory.TypeNamespace(), factory.TypeName(),
        method_name,
        bind<write_consume_args>(signature));

}

void write_static_definitions(writer& w, TypeDef const& type)
{
    for (auto&& factory : get_static_factories(type))
    {
        w.write_each<write_static_definition>(factory.MethodList(), type.TypeName(), factory);
    }
}

void write_class_usings(writer& w, TypeDef const& type)
{

}

void write_class(writer& w, TypeDef const& type)
{
    auto guard{ w.push_generic_params(type.GenericParam()) };
    auto type_name = type.TypeName();

    if (auto default_interface = get_default_interface(type))
    {
        w.write(
R"(
struct WINRT_EBO % :
    %%%
{
    %(std::nullptr_t) noexcept {}
%%%};
)",
            type_name,
            default_interface,
            bind<write_class_base>(type),
            bind<write_class_requires>(type),
            type_name,
            bind<write_constructor_declarations>(type),
            bind<write_class_usings>(type),
            bind<write_static_declarations>(type));
    }
    else
    {
        w.write(
            R"(
struct %
{
    %() = delete;
%};
)",
            type_name,
            type_name,
            bind<write_static_declarations>(type));
    }
}

auto get_in(cmd::reader const& args)
{
    std::vector<std::string> files;

    auto add_directory = [&](auto&& path)
    {
        for (auto&& file : directory_iterator(path))
        {
            if (is_regular_file(file))
            {
                files.push_back(file.path().string());
            }
        }
    };

    for (auto&& path : args.values("input"))
    {
        if (is_directory(path))
        {
            add_directory(path);
        }
        else if (is_regular_file(path))
        {
            files.push_back(path);
        }
#if XLANG_PLATFORM_WINDOWS
        else if (path == "local")
        {
            std::array<char, MAX_PATH> local{};
            ExpandEnvironmentStringsA("%windir%\\System32\\WinMetadata", local.data(), static_cast<DWORD>(local.size()));
            add_directory(local.data());
        }
#endif
        else
        {
            throw_invalid("Path '", path, "' is not a file or directory");
        }
    }

    return files;
}

auto get_output(cmd::reader const& args)
{
    auto output{ absolute(args.value("output", "winrt")) };
    create_directories(output / "impl");
    output += path::preferred_separator;
    return output.string();
}

void print_usage()
{
    puts("Usage...");
}

int main(int const argc, char** argv)
{
    writer w;

    try
    {
        auto start = high_resolution_clock::now();

        std::vector<cmd::option> options
        {
            // name, min, max
            { "input", 1 },
            { "output", 0, 1 },
            { "include", 0 },
            { "exclude", 0 },
            { "verbose", 0, 0 },
        };

        cmd::reader args{ argc, argv, options };

        if (!args)
        {
            print_usage();
            return 0;
        }

        cache c{ get_in(args) };
        c.remove_legacy_cppwinrt_foundation_types();
        auto const output_folder = get_output(args);
        bool const verbose = args.exists("verbose");

        auto relative_folder = path{ output_folder.substr(0, output_folder.size() - 1) }.filename().string();

        filter f;
        f.include(args.values("include"));
        f.exclude(args.values("exclude"));

        if (verbose)
        {
            for (auto&& db : c.databases())
            {
                w.write("input: %\n", db.path());
            }

            w.write("output: %\n", output_folder);
        }

        w.flush_to_console();
        task_group group;

        group.add([&]
        {
            writer w;
            w.write_license();
            w.write_include_guard();

            w.write(strings::base_dependencies);
            w.write(strings::base_macros);
            w.write(strings::base_extern);
            w.write(strings::base_forward);
            w.write(strings::base_meta);
            w.write(strings::base_identity);
            w.write(strings::base_handle);
            w.write(strings::base_lock);
            w.write(strings::base_diagnostics);
            w.write(strings::base_abi);
            w.write(strings::base_windows);
            w.write(strings::base_string);
            w.write(strings::base_string_input);
            w.write(strings::base_string_operators);
            w.write(strings::base_array);
            w.write(strings::base_com_ptr);
            w.write(strings::base_weak_ref);
            w.write(strings::base_agile_ref);
            w.write(strings::base_error);
            w.write(strings::base_events);
            w.write(strings::base_consume);
            w.write(strings::base_traits);
            w.write(strings::base_marshaler);
            w.write(strings::base_delegate);
            w.write(strings::base_types);
            w.write(strings::base_shims);
            w.write(strings::base_activation);
            w.write(strings::base_implements);
            w.write(strings::base_produce);
            w.write(strings::base_composable);
            w.write(strings::base_foundation);
            w.write(strings::base_chrono);
            w.write(strings::base_security);
            w.write(strings::base_await);
            w.write(strings::base_collections);
            w.write(strings::base_collections_base);
            w.write(strings::base_collections_input_iterable);
            w.write(strings::base_collections_input_vector_view);
            w.write(strings::base_collections_input_map_view);
            w.write(strings::base_collections_input_vector);
            w.write(strings::base_collections_input_map);
            w.write(strings::base_collections_vector);
            w.write(strings::base_collections_map);
            w.write(strings::base_std_hash);
            w.write(strings::base_std_fire_and_forget);
            w.write(strings::base_std_async_action);
            w.write(strings::base_std_async_action_with_progress);
            w.write(strings::base_std_async_operation);
            w.write(strings::base_std_async_operation_with_progress);

            w.write(strings::base_reflect);

            w.write(strings::base_natvis);
            w.write(strings::base_version, XLANG_VERSION_STRING);

            w.flush_to_file(output_folder + "base.h");
        });

        for (auto&& ns : c.namespaces())
        {
            group.add([&]
            {
                if (!f.match(ns.second))
                {
                    return;
                }

                {
                    writer w;
                    w.type_namespace = ns.first;
                    w.output_folder = output_folder;
                    w.relative_folder = relative_folder;

                    write_type_namespace(w, ns.first);
                    w.write_each<write_enum>(ns.second.enums);
                    w.write("\n");
                    w.write_each<write_forward>(ns.second.interfaces);
                    w.write_each<write_forward>(ns.second.classes);
                    w.write_each<write_forward>(ns.second.structs);
                    w.write_each<write_forward>(ns.second.delegates);
                    write_close_namespace(w);
                    write_impl_namespace(w);
                    w.write("\n");
                    w.write_each<write_enum_flag>(ns.second.enums);
                    w.write_each<write_category>(ns.second.interfaces, "interface_category");
                    w.write_each<write_category>(ns.second.classes, "class_category");
                    w.write_each<write_category>(ns.second.enums, "enum_category");
                    w.write_each<write_struct_category>(ns.second.structs);
                    w.write_each<write_category>(ns.second.delegates, "delegate_category");

                    w.write_each<write_name>(ns.second.interfaces);
                    w.write_each<write_name>(ns.second.classes);
                    w.write_each<write_name>(ns.second.enums);
                    w.write_each<write_name>(ns.second.structs);
                    w.write_each<write_name>(ns.second.delegates);

                    w.write_each<write_guid>(ns.second.interfaces);
                    w.write_each<write_guid>(ns.second.delegates);
                    w.write_each<write_default_interface>(ns.second.classes);

                    w.abi_types = true;
                    w.write_each<write_interface_abi>(ns.second.interfaces);
                    w.write_each<write_delegate_abi>(ns.second.delegates);
                    w.abi_types = false;

                    w.write_each<write_consume>(ns.second.interfaces);

                    // TODO: Move to the abi_types group above after validation against cppwinrt.exe
                    w.abi_types = true;
                    w.write_each<write_struct_abi>(ns.second.structs);
                    w.abi_types = false;

                    write_close_namespace(w);

                    w.swap();
                    w.write_license();
                    w.write_include_guard();
                    w.write_depends("base");

                    for (auto&& ns : w.depends)
                    {
                        write_type_namespace(w, ns.first);
                        w.write("\n");
                        w.write_each<write_forward>(ns.second);
                        write_close_namespace(w);
                    }

                    w.save_header('0');
                }
                {
                    writer w;
                    w.type_namespace = ns.first;
                    w.output_folder = output_folder;
                    w.relative_folder = relative_folder;

                    write_type_namespace(w, ns.first);
                    w.write_each<write_interface>(ns.second.interfaces);
                    write_close_namespace(w);

                    w.swap();
                    w.write_license();
                    w.write_include_guard();

                    for (auto&& ns : w.depends)
                    {
                        w.write_depends(ns.first, '0');
                    }

                    w.write_depends(w.type_namespace, '0');
                    w.save_header('1');
                }
                {
                    writer w;
                    w.type_namespace = ns.first;
                    w.output_folder = output_folder;
                    w.relative_folder = relative_folder;

                    w.write_license();
                    w.write_include_guard();
                    w.write_depends(w.type_namespace, '1');
                    write_type_namespace(w, ns.first);
                    w.write_each<write_delegate>(ns.second.delegates);
                    write_structs(w, ns.second.structs);
                    w.write_each<write_class>(ns.second.classes);
                    // write interface overrides
                    write_close_namespace(w);
                    w.save_header('2');
                }
                {
                    writer w;
                    w.type_namespace = ns.first;
                    w.output_folder = output_folder;
                    w.relative_folder = relative_folder;

                    w.write_license();
                    w.write_include_guard();
                    w.write_depends(w.type_namespace, '2');
                    write_impl_namespace(w);
                    w.write_each<write_consume_definitions>(ns.second.interfaces);
                    w.write_each<write_delegate_implementation>(ns.second.delegates);
                    w.write_each<write_produce>(ns.second.interfaces);
                    write_close_namespace(w);

                    write_type_namespace(w, ns.first);
                    w.write_each<write_constructor_definitions>(ns.second.classes);
                    // write composable constructors
                    w.write_each<write_static_definitions>(ns.second.classes);
                    write_close_namespace(w);
                    w.save_header();
                }
            });
        }

        group.get();

        if (verbose)
        {
            w.write("time: %ms\n", duration_cast<duration<int64_t, std::milli>>(high_resolution_clock::now() - start).count());
        }
    }
    catch (std::exception const& e)
    {
        w.write("%\n", e.what());
    }

    w.flush_to_console();
}
