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
                [&](uint32_t var)
                {
                    write("?%?", var);
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
};

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
    // TODO: since attributs are so important to IDL, we should probably have an IDL attribute writer
    // that walks all the attributes and prints the complete IDL representation...
    if (type.has_attribute("System", "FlagsAttribute"))
    {
        w.write("\n    [flags]");
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
    w.write("\n    struct %\n    {%\n    };\n",
        type.TypeName(),
        bind_each<write_struct_field>(type.FieldList()));
}

void write_delegate(writer& w, TypeDef const& type)
{
    auto methods = type.MethodList();

    auto method = std::find_if(begin(methods), end(methods), [](auto&& method)
    {
        return method.Name() == "Invoke";
    });

    if (method == end(methods))
    {
        xlang::throw_invalid("Delegate's Invoke method not found");
    }

    w.write("\n    delegate % %(%);\n", method.Signature().ReturnType(), type.TypeName(), method);
}

void write_method(writer& w, MethodDef const& method)
{
    w.write("\n        % %(%);", method.Signature().ReturnType(), method.Name(), method);
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

void write_interface(writer& w, TypeDef const& type)
{
    w.write("\n    interface %%\n    {%\n    };\n",
        type.TypeName(),
        bind<write_required>("requires", type),
        bind_each<write_method>(type.MethodList()));
}

void write_class(writer& w, TypeDef const& type)
{
    w.write("\n    runtimeclass %%\n    {\n    };\n",
        type.TypeName(),
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
