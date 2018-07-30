#include "pch.h"

using namespace std::chrono;
using namespace std::filesystem;
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
        generic_param_stack.push_back(std::move(arg));
        return generic_param_guard{ this };
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
        write("%.%", type.TypeNamespace(), type.TypeName());
    }

    void write(TypeRef const& type)
    {
        write("%.%", type.TypeNamespace(), type.TypeName());
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
        write("%<%>",
            type.GenericType(),
            bind_list(", ", type.GenericArgs()));
    }

    void write(TypeSig const& signature)
    {
        std::visit(overloaded
            {
                [&](ElementType type)
                {
                    if (type <= ElementType::String)
                    {
                        static constexpr const char* primitives[]
                        {
                            "end",
                            "void",
                            "boolean",
                            "char",
                            "int8",
                            "uint8",
                            "int16",
                            "uint16",
                            "int32_t",
                            "uint32_t",
                            "int64",
                            "uint64",
                            "single",
                            "double",
                            "string"
                        };

                        write(primitives[static_cast<uint32_t>(type)]);
                    }
                    else if (type == ElementType::Object)
                    {
                        write("Object");
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

    void write(MethodDef const& method)
    {
        auto signature{ method.Signature() };
        
        auto param = begin(method.ParamList());

        if (method.Signature().ReturnType().Type() && param.Sequence() == 0)
        {
            ++param;
        }

        auto is_const = [](ParamSig const& param) -> bool
        {
            for (auto const& cmod : param.CustomMod())
            {
                switch (cmod.Type().type())
                {
                case TypeDefOrRef::TypeRef:
                {
                    auto const& type = cmod.Type().TypeRef();
                    return type.TypeNamespace() == "System.Runtime.CompilerServices" && type.TypeName() == "IsConst";
                }
                case TypeDefOrRef::TypeDef:
                {
                    auto const& type = cmod.Type().TypeDef();
                    return type.TypeNamespace() == "System.Runtime.CompilerServices" && type.TypeName() == "IsConst";
                }
                }
            }
            return false;
        };

        bool first{ true };

        for (auto&& arg : signature.Params())
        {
            if (first)
            {
                first = false;
            }
            else
            {
                write(", ");
            }

            if (arg.ByRef())
            {
                write("ref ");
            }

            if (is_const(arg))
            {
                write("const ");
            }

            write("% %", arg.Type(), param.Name());
            ++param;
        }
    }

    void write(RetTypeSig const& signature)
    {
        if (auto type = signature.Type())
        {
            write(*type);
        }
        else
        {
            write("void");
        }
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

    void write_uuid(CustomAttributeSig const& arg)
    {
        auto const& args = arg.FixedArgs();

        write_printf("\n    [Windows.Foundation.Metadata.GuidAttribute(%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X)]"
            , std::get<uint32_t>(std::get<ElemSig>(args[0].value).value)
            , std::get<uint16_t>(std::get<ElemSig>(args[1].value).value)
            , std::get<uint16_t>(std::get<ElemSig>(args[2].value).value)
            , std::get<uint8_t>(std::get<ElemSig>(args[3].value).value)
            , std::get<uint8_t>(std::get<ElemSig>(args[4].value).value)
            , std::get<uint8_t>(std::get<ElemSig>(args[5].value).value)
            , std::get<uint8_t>(std::get<ElemSig>(args[6].value).value)
            , std::get<uint8_t>(std::get<ElemSig>(args[7].value).value)
            , std::get<uint8_t>(std::get<ElemSig>(args[8].value).value)
            , std::get<uint8_t>(std::get<ElemSig>(args[9].value).value)
            , std::get<uint8_t>(std::get<ElemSig>(args[10].value).value)
        );
    }

    void write(CustomAttribute const& attr)
    {
        auto const& name = attr.TypeNamespaceAndName();
        auto const& sig = attr.Value();

        if (name.first == "Windows.Foundation.Metadata"sv && name.second == "GuidAttribute"sv)
        {
            write_uuid(sig);
            return;
        }

        write("\n    [%.%", name.first, name.second);

        bool first = true;
        for (auto const& fixed_arg : sig.FixedArgs())
        {
            if (first)
            {
                first = false;
                write("(%", fixed_arg);
            }
            else
            {
                write(", %", fixed_arg);
            }
        }
        for (auto const& named_arg : sig.NamedArgs())
        {
            if (first)
            {
                first = false;
                write("(%", named_arg);
            }
            else
            {
                write(", %", named_arg);
            }
        }
        if (first)
        {
            write("]");
        }
        else
        {
            write(")]");
        }
    }
};

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

void write_type_name(writer& w, std::string_view const& name)
{
    w.write(name);
    bool found = false;
    for (auto const& param : w.generic_param_stack.back())
    {
        if (found)
        {
            w.write(", %", param.Name());
        }
        else
        {
            found = true;
            w.write("<%", param.Name());
        }
    }
    if (found)
    {
        w.write(">");
    }
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

    w.write("\nenum class % : %\n{%\n};\n",
        type.TypeName(),
        field.Signature().Type(),
        bind_each<write_enum_field>(type.FieldList()));
}

void write_forward(writer& w, TypeDef const& type)
{
    w.write("struct %;\n", type.TypeName());
}

void write_enum_flag(writer& w, TypeDef const& type)
{
    if (type.has_attribute("System", "FlagsAttribute"))
    {
        w.write("template<> struct is_enum_flag<@::%> : std::true_type {};\n",
            type.TypeNamespace(),
            type.TypeName());
    }
}

void write_category(writer& w, TypeDef const& type, std::string_view const& category)
{
    w.write("template <> struct category<@::%>{ using type = %; };\n",
        type.TypeNamespace(),
        type.TypeName(),
        category);
}

void write_name(writer& w, TypeDef const& type)
{
    auto ns = type.TypeNamespace();
    auto name = type.TypeName();

    w.write("template <> struct name<@::%>{ static constexpr auto & value{ L\"%.%\" }; };\n",
        ns, name, ns, name);
}

void write_field_types(writer& w, TypeDef const& type)
{
    bool first{ true };

    for (auto&& field : type.FieldList())
    {
        if (first)
        {
            first = false;
        }
        else
        {
            w.write(", ");
        }

        w.write(field.Signature().Type());
    }
}

void write_struct_category(writer& w, TypeDef const& type)
{
    w.write("template <> struct category<@::%>{ using type = struct_category<%>; };\n",
        type.TypeNamespace(),
        type.TypeName(),
        bind<write_field_types>(type));
}

void write_struct_field(writer& w, Field const& field)
{
    w.write("\n        % %;",
        field.Signature().Type(),
        field.Name());
}

void write_struct(writer& w, TypeDef const& type)
{
    for (auto const& attr : type.CustomAttribute())
    {
        w.write(attr);
    }

    w.write("\n    struct %\n    {%\n    };\n",
        type.TypeName(),
        bind_each<write_struct_field>(type.FieldList()));
}

void write_delegate(writer& w, TypeDef const& type)
{
    auto guard{ w.push_generic_params(type.GenericParam()) };
    auto methods = type.MethodList();

    auto method = std::find_if(begin(methods), end(methods), [](auto&& method)
    {
        return method.Name() == "Invoke";
    });

    if (method == end(methods))
    {
        throw_invalid("Delegate's Invoke method not found");
    }

    for (auto const& attr : type.CustomAttribute())
    {
        w.write(attr);
    }

    w.write("\n    delegate % %(%);\n", method.Signature().ReturnType(), bind<write_type_name>(type.TypeName()), method);
}

void write_method(writer& w, MethodDef const& method)
{
    for (auto const& attr : method.CustomAttribute())
    {
        w.write(attr);
    }

    w.write("\n        % %(%);", method.Signature().ReturnType(), method.Name(), method);
}

void write_method_semantic(writer& w, MethodSemantics const& method)
{
    switch (method.Semantic())
    {
    case MethodSemanticsAttributes::Getter:
        w.write("\n        % % { get; };", method.Association().Property().Type().Type(), method.Association().Property().Name());
        break;

    case MethodSemanticsAttributes::Setter:
        w.write("\n        % % { set; };", method.Association().Property().Type().Type(), method.Association().Property().Name());
        break;

    case MethodSemanticsAttributes::AddOn:
        w.write("\n        % % { add; };", method.Association().Event().EventType(), method.Association().Event().Name());
        break;

    case MethodSemanticsAttributes::RemoveOn:
        w.write("\n        % % { remove; };", method.Association().Event().EventType(), method.Association().Event().Name());
        break;

    default:
        throw_invalid("Invalid method semantic encountered");
    }
}

void write_required(writer& w, std::string_view const& requires, TypeDef const& type)
{
    auto interfaces{ type.InterfaceImpl() };

    if (begin(interfaces) == end(interfaces))
    {
        return;
    }

    w.write(" %\n        %", requires, bind_list(",\n        ", interfaces));
}

void write_interface_methods(writer& w, TypeDef const& type)
{
    auto const& methods = type.MethodList();
    auto const& properties = type.PropertyList();
    auto const& events = type.EventList();

    auto method_semantic = [&properties, &events](MethodDef const& method) -> std::optional<MethodSemantics>
    {
        for (auto const& prop : properties)
        {
            for (auto const& semantic : prop.MethodSemantic())
            {
                if (semantic.Method() == method)
                {
                    return semantic;
                }
            }
        }
        for (auto const& event : events)
        {
            for (auto const& semantic : event.MethodSemantic())
            {
                if (semantic.Method() == method)
                {
                    return semantic;
                }
            }
        }
        return {};
    };

    for (auto const& method : methods)
    {
        auto const& semantic = method_semantic(method);
        if (semantic)
        {
            write_method_semantic(w, *semantic);
        }
        else
        {
            write_method(w, method);
        }
    }
}

void write_interface(writer& w, TypeDef const& type)
{
    auto guard = w.push_generic_params(type.GenericParam());

    for (auto const& attr : type.CustomAttribute())
    {
        w.write(attr);
    }

    w.write("\n    interface %%\n    {%\n    };\n",
        bind<write_type_name>(type.TypeName()),
        bind<write_required>("requires", type),
        bind<write_interface_methods>(type));
}

void write_class(writer& w, TypeDef const& type)
{
    auto guard = w.push_generic_params(type.GenericParam());

    for (auto const& attr : type.CustomAttribute())
    {
        w.write(attr);
    }

    w.write("\n    runtimeclass %%\n    {\n    };\n",
        bind<write_type_name>(type.TypeName()),
        bind<write_required>(":", type));
}

auto get_in(reader const& args)
{
    std::vector<std::string> files;

    auto add_directory = [&](auto&& path)
    {
        for (auto&& file : directory_iterator(path))
        {
            if (file.is_regular_file())
            {
                files.push_back(file.path().string());
            }
        }
    };

    for (auto&& path : args.values("in"))
    {
        if (is_directory(path))
        {
            add_directory(path);
        }
        else if (is_regular_file(path))
        {
            files.push_back(path);
        }
        else if (path == "local")
        {
            std::array<char, MAX_PATH> local{};
            ExpandEnvironmentStringsA("%windir%\\System32\\WinMetadata", local.data(), static_cast<DWORD>(local.size()));
            add_directory(local.data());
        }
        else
        {
            throw_invalid("Path '", path, "' is not a file or directory");
        }
    }

    return files;
}

auto get_out(reader const& args)
{
    auto out{ absolute(args.value("out", "idl")) };
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
            { "in", 1 },
            { "out", 0, 1 },
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

                auto filename{ out };
                filename += "impl";
                filename += static_cast<char>(path::preferred_separator);
                filename += ns.first;
                filename += ".0.h";
                w.flush_to_file(filename);
            });

            group.add([&]
            {
                writer w;

                w.write(".1");

                auto filename{ out };
                filename += "impl";
                filename += static_cast<char>(path::preferred_separator);
                filename += ns.first;
                filename += ".1.h";
                w.flush_to_file(filename);
            });

            group.add([&]
            {
                writer w;

                w.write(".2");

                auto filename{ out };
                filename += "impl";
                filename += static_cast<char>(path::preferred_separator);
                filename += ns.first;
                filename += ".2.h";
                w.flush_to_file(filename);
            });

            group.add([&]
            {
                writer w;

                w.write("top");

                auto filename{ out };
                filename += ns.first;
                filename += ".h";
                w.flush_to_file(filename);
            });
        }

        group.get();

        if (verbose)
        {
            w.write("time: %ms\n", duration_cast<milliseconds>(high_resolution_clock::now() - start).count());
        }
    }
    catch (std::exception const& e)
    {
        w.write("%\n", e.what());
    }

    w.flush_to_console();
}
