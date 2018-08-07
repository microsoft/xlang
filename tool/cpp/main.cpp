#include "pch.h"

using namespace std::chrono;
using namespace std::experimental::filesystem;
using namespace std::string_view_literals;
using namespace xlang;
using namespace xlang::meta::reader;
using namespace xlang::text;
using namespace xlang::cmd;

template <typename...T> struct overloaded : T... { using T::operator()...; };
template <typename...T> overloaded(T...)->overloaded<T...>;

struct writer : writer_base<writer>
{
    using writer_base<writer>::write;

    bool abi_types{};
    bool abi_definitions{};
    bool param_types{};
    bool async_types{};

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

    void write(TypeDef const& type)
    {
        write("@::%", type.TypeNamespace(), type.TypeName());
    }

    void write(TypeRef const& type)
    {
        write("@::%", type.TypeNamespace(), type.TypeName());
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
        else if (param_types)
        {
            static constexpr std::string_view optional("Windows.Foundation.IReference"sv);
            static constexpr std::string_view iterable("Windows.Foundation.Collections.IIterable"sv);
            static constexpr std::string_view vector_view("Windows.Foundation.Collections.IVectorView"sv);
            static constexpr std::string_view map_view("Windows.Foundation.Collections.IMapView"sv);
            static constexpr std::string_view vector("Windows.Foundation.Collections.IVector"sv);
            static constexpr std::string_view map("Windows.Foundation.Collections.IMap"sv);

            auto name = write_temp("%<%>", type.GenericType(), bind_list(", ", type.GenericArgs()));

            if (starts_with(name, optional))
            {
                write("optional@", name.substr(optional.size()));
            }
            else if (starts_with(name, iterable))
            {
                if (async_types)
                {
                    write("param::async_iterable@", name.substr(iterable.size()));
                }
                else
                {
                    write("param::iterable@", name.substr(iterable.size()));
                }
            }
            else if (starts_with(name, vector_view))
            {
                if (async_types)
                {
                    write("param::async_vector_view@", name.substr(vector_view.size()));
                }
                else
                {
                    write("param::vector_view@", name.substr(vector_view.size()));
                }
            }

            else if (starts_with(name, map_view))
            {
                if (async_types)
                {
                    write("param::async_map_view@", name.substr(map_view.size()));
                }
                else
                {
                    write("param::map_view@", name.substr(map_view.size()));
                }
            }
            else if (starts_with(name, vector))
            {
                write("param::vector@", name.substr(vector.size()));
            }
            else if (starts_with(name, map))
            {
                write("param::map@", name.substr(vector.size()));
            }
            else
            {
                write(name);
            }
        }
        else
        {
            write("%<%>",
                type.GenericType(),
                bind_list(", ", type.GenericArgs()));
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
                        else if (param_types)
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

    auto const& return_param() const noexcept
    {
        return m_return;
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

bool is_noexcept(MethodDef const& method)
{
    return (enum_mask(method.Flags(), MethodAttributes::SpecialName) == MethodAttributes::SpecialName && starts_with(method.Name(), "remove_")) ||
        get_attribute(method, "Experimental.Windows.Foundation.Metadata", "NoExceptAttribute");
}

void write_impl_namespace(writer& w)
{
    w.write("\nnamespace winrt::impl {\n");
}

void write_std_namespace(writer& w)
{
    w.write("\nWINRT_EXPORT namespace std {\n");
}

void write_meta_namespace(writer& w, std::string_view const& ns)
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
    Field const& field{ *type.FieldList().first };
    auto name = field.Name();

    w.write(
R"(
enum class % : %
{%
};
)",
        type.TypeName(),
        field.Signature().Type(),
        bind_each<write_enum_field>(type.FieldList()));
}

void write_forward(writer& w, TypeDef const& type)
{
    w.write("struct @;\n", type.TypeName());
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
    if (w.abi_definitions)
    {
        w.write(" __%Size", param.Name());
    }
}

void write_abi_params(writer& w, MethodDef const& method)
{
    separator s{ w };
    method_signature method_signature{ method };

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

        if (w.abi_definitions)
        {
            w.write(" %", param.Name());
        }
    }

    if (auto return_signature = method_signature.return_signature())
    {
        s();

        auto const& type = return_signature.Type();

        if (type.is_szarray())
        {
            w.write("uint32_t, %*", type);
        }
        else
        {
            w.write("%*", type);
        }

        if (w.abi_definitions)
        {
            w.write(" %", method_signature.return_param().Name());
        }
    }
}

void write_abi_declaration(writer& w, MethodDef const& method)
{
    w.write("    virtual int32_t WINRT_CALL %(%) noexcept = 0;\n",
        get_abi_name(method),
        bind<write_abi_params>(method));
}

void write_interface_abi(writer& w, TypeDef const& type)
{
    auto guard{ w.push_generic_params(type.GenericParam()) };

    w.write(
R"(
template <> struct abi<%>{ struct type : IInspectable
{
%};};
)",
        type,
        bind_each<write_abi_declaration>(type.MethodList()));
}

//void write_delegate_abi(writer& w, TypeDef const& type)
//{
//
//}

void write_consume_params(writer& w, method_signature const& method_signature)
{
    separator s{ w };

    for (auto&&[param, param_signature] : method_signature.params())
    {
        s();

        if (is_in(param))
        {
            XLANG_ASSERT(!is_out(param));

            w.param_types = true;
            w.write("% const&", param_signature.Type());
            w.param_types = false;
        }
        else
        {
            XLANG_ASSERT(!is_in(param));
            XLANG_ASSERT(is_out(param));

            w.write("%&", param_signature.Type());
        }

        w.write(" %", param.Name());
    }
}

void write_consume_declaration(writer& w, MethodDef const& method)
{
    method_signature signature{ method };

    w.write("    % %(%) const%;\n",
        signature.return_signature(),
        get_name(method),
        bind<write_consume_params>(signature),
        is_noexcept(method) ? " noexcept" : "");
}

void write_impl_name(writer& w, TypeDef const& type)
{
    for (auto&& c : type.TypeNamespace())
    {
        w.write(c == '.' ? '_' : c);
    }

    w.write('_');

    for (auto&& c : type.TypeName())
    {
        w.write(c == '.' ? '_' : c);
    }
}

void write_consume(writer& w, TypeDef const& type)
{
    auto guard{ w.push_generic_params(type.GenericParam()) };

    w.write(
R"(
template <typename D>\nstruct consume_%
{
%%};
template <> struct consume<%> { template <typename D> using type = consume_%<D>; };
)",
        bind<write_impl_name>(type),
        bind_each<write_consume_declaration>(type.MethodList()),
        "", // extensions?
        type,
        bind<write_impl_name>(type));
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
        "<usings>");
}

auto get_in(reader const& args)
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

auto get_out(reader const& args)
{
    auto out{ absolute(args.value("output", "winrt")) };
    create_directories(out / "impl");
    out += path::preferred_separator;
    return out.string();
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

        std::vector<option> options
        {
            // name, min, max
            { "input", 1 },
            { "output", 0, 1 },
            { "include", 0 },
            { "exclude", 0 },
            { "verbose", 0, 0 },
        };

        reader args{ argc, argv, options };

        if (!args)
        {
            print_usage();
            return 0;
        }

        cache c{ get_in(args) };
        auto const out = get_out(args);
        bool const verbose = args.exists("verbose");

        filter f;
        f.include(args.values("include"));
        f.exclude(args.values("exclude"));

        if (verbose)
        {
            std::for_each(c.databases().begin(), c.databases().end(), [&](auto&& db)
            {
                w.write("in: %\n", db.path());
            });

            w.write("out: %\n", out);
        }

        w.flush_to_console();
        task_group group;

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
                    write_meta_namespace(w, ns.first);
                    w.write_each<write_enum>(ns.second.enums);
                    w.write_each<write_forward>(ns.second.interfaces);
                    w.write_each<write_forward>(ns.second.classes);
                    w.write_each<write_forward>(ns.second.structs);
                    w.write_each<write_forward>(ns.second.delegates);
                    write_close_namespace(w);
                    write_impl_namespace(w);
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
                    //w.write_each<write_delegate_abi>(ns.second.delegates);
                    w.abi_types = false;

                    w.write_each<write_consume>(ns.second.interfaces);
                    write_close_namespace(w);

                    auto filename{ out };
                    filename += "impl";
                    filename += static_cast<char>(path::preferred_separator);
                    filename += ns.first;
                    filename += ".0.h";
                    w.flush_to_file(filename);
                }
                {
                    writer w;
                    write_meta_namespace(w, ns.first);
                    w.write_each<write_interface>(ns.second.interfaces);
                    write_close_namespace(w);

                    auto filename{ out };
                    filename += "impl";
                    filename += static_cast<char>(path::preferred_separator);
                    filename += ns.first;
                    filename += ".1.h";
                    w.flush_to_file(filename);
                }
                {
                    writer w;

                    w.write(".2");

                    auto filename{ out };
                    filename += "impl";
                    filename += static_cast<char>(path::preferred_separator);
                    filename += ns.first;
                    filename += ".2.h";
                    w.flush_to_file(filename);
                }
                {
                    writer w;

                    w.write("top");

                    auto filename{ out };
                    filename += ns.first;
                    filename += ".h";
                    w.flush_to_file(filename);
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
