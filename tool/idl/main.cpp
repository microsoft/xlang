#include "pch.h"
#include "meta_reader.h"
#include "text_writer.h"
#include "cmd_reader.h"

using namespace std::chrono;
using namespace std::filesystem;
using namespace winrt::Windows::Foundation;
using namespace xlang::meta::reader;
using namespace xlang::text;
using namespace xlang::cmd;

template <typename...T> struct overloaded : T... { using T::operator()...; };
template <typename...T> overloaded(T...)->overloaded<T...>;

struct writer : writer_base<writer>
{
    using writer_base<writer>::write;
    std::string_view current;

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

    void write(Constant const& value)
    {
        auto const type = value.Type();

        if (type == ConstantType::Int32)
        {
            write_printf("%d", value.ValueInt32());
        }
        else if (type == ConstantType::UInt32)
        {
            write_printf("0x%X", value.ValueUInt32());
        }
    }

    void write(TypeDef const& type)
    {
        write("%.%", type.TypeNamespace(), type.TypeName());
    }

    void write(TypeRef const& type)
    {
        auto ns = type.TypeNamespace();

        if (ns == current)
        {
            write("%", type.TypeName());
        }
        else
        {
            write("%.%", type.TypeNamespace(), type.TypeName());
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
                            "End",
                            "Void",
                            "Boolean",
                            "Char",
                            "Int8",
                            "UInt8",
                            "Int16",
                            "UInt16",
                            "Int32",
                            "UInt32",
                            "Int64",
                            "UInt64",
                            "Single",
                            "Double",
                            "String"
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

    void write(CustomAttribute const& attr)
    {
        auto const& name = attr.TypeNamespaceAndName();
        write("\n    [%.%]", name.first, name.second);
    }
};

void write_type_name(writer& w, std::string_view name)
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
        w.write("\n        % = %,",
            field.Name(),
            *constant);
    }
}

void write_enum(writer& w, TypeDef const& type)
{
    for (auto const& attr : type.CustomAttribute())
    {
        w.write(attr);
    }

    w.write("\n    enum %\n    {%\n    };\n",
        type.TypeName(),
        bind_each<write_enum_field>(type.FieldList()));
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
        xlang::throw_invalid("Delegate's Invoke method not found");
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
        xlang::throw_invalid("Invalid method semantic encountered");
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

IAsyncAction write_namespace(cache const& c, std::string_view const& name, cache::namespace_members const& members)
{
    co_await winrt::resume_background();
    writer w;
    w.current = name;

    w.write("\nnamespace %\n{%%%%%}\n",
        name,
        bind_each<write_enum>(members.enums),
        bind_each<write_struct>(members.structs),
        bind_each<write_delegate>(members.delegates),
        bind_each<write_interface>(members.interfaces),
        bind_each<write_class>(members.classes));

    std::string path{ R"(C:\git\xlang\output\idl\)" };
    path += name;
    path += ".idl";
    w.save_to_file(path);
}

auto directory_vector(reader const& args)
{
    std::vector<std::string> files;

    for (auto&& path : args.values("in"))
    {
        if (is_directory(path))
        {
            for (auto&& file : directory_iterator(path))
            {
                if (file.is_regular_file())
                {
                    files.push_back(file.path().string());
                }
            }
        }
        else if (is_regular_file(path))
        {
            files.push_back(path);
        }
        else
        {
            xlang::throw_invalid("Path '", path, "' is not a file or directory");
        }
    }
    return files;
}

void print_usage()
{
    puts("Usage...");
}

int main(int const argc, char** argv)
{
    try
    {
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

        auto start = high_resolution_clock::now();
        winrt::init_apartment();
        cache c{ directory_vector(args) };
        std::vector<IAsyncAction> actions;

        for (auto&& ns : c.namespaces())
        {
            actions.push_back(write_namespace(c, ns.first, ns.second));
        }

        for (auto&& async : actions)
        {
            async.get();
        }

        printf("Time: %llums\n", duration_cast<milliseconds>(high_resolution_clock::now() - start).count());
    }
    catch (winrt::hresult_error const& e)
    {
        printf("%ls\n", e.message().c_str());
    }
    catch (std::exception const& e)
    {
        printf("%s\n", e.what());
    }
}
