#pragma once

namespace xlang
{
    static void write_consumer_args(writer& w, method_signature const& signature)
    {
        separator s{ w };

        for (auto&& [param, param_signature] : signature.params())
        {
            s();

            if (param_signature->Type().is_szarray())
            {
                std::string_view format;

                if (param.Flags().In())
                {
                    format = "arg<array_view<% const>>()";
                }
                else if (param_signature->ByRef())
                {
                    format = "arg<com_array<%>&>()";
                }
                else
                {
                    format = "arg<array_view<%>>()";
                }

                w.write(format, param_signature->Type().Type());
            }
            else
            {
                if (param.Flags().In())
                {
                    XLANG_ASSERT(!param.Flags().Out());

                    auto param_type = std::get_if<ElementType>(&param_signature->Type().Type());

                    if (param_type && *param_type != ElementType::String && *param_type != ElementType::Object)
                    {
                        w.write("arg<%>()", param_signature->Type());
                    }
                    else if (std::holds_alternative<GenericTypeIndex>(param_signature->Type().Type()))
                    {
                        w.write("arg<% const&>()", param_signature->Type());
                    }
                    else
                    {
                        w.write("arg<% const&>()", param_signature->Type());
                    }
                }
                else
                {
                    XLANG_ASSERT(!param.Flags().In());
                    XLANG_ASSERT(param.Flags().Out());

                    w.write("arg<%&>()", param_signature->Type());
                }
            }
        }
    }

    static void write_consumer_method(writer& w, MethodDef const& method)
    {
        method_signature signature{ method };
        auto format = R"(    o.%(%);
)";

        w.write(format,
            get_name(method),
            bind<write_consumer_args>(signature));
    }

    static void write_consumer_method_list(writer& w, TypeDef const& type)
    {
        auto format = R"(    // %
%)";

        w.write(format,
            type,
            bind_each<write_consumer_method>(type.MethodList()));
    }


    static void write_consumer_interfaces(writer& w, get_interfaces_t const& interfaces)
    {
        for (auto&& [name, info] : interfaces)
        {
            w.generic_param_stack.insert(w.generic_param_stack.end(), info.generic_param_stack.begin(), info.generic_param_stack.end());

            write_consumer_method_list(w, info.type);

            w.generic_param_stack.resize(w.generic_param_stack.size() - info.generic_param_stack.size());
        }
    }

    static void write_consumer_class(writer& w, TypeDef const& type)
    {
        auto generics = type.GenericParam();
        auto guard{ w.push_generic_params(generics) };

        // TODO: add calls to constructors and statics

        if (!get_default_interface(type))
        {
            return;
        }

        auto format = R"(
void %()
{
    % o{ nullptr };
%}
)";

        w.write(format,
            get_impl_name(type.TypeNamespace(), type.TypeName()),
            type,
            bind<write_consumer_interfaces>(get_interfaces(w, type)));
    }

    static void write_consumer_interface(writer& w, TypeDef const& type)
    {
        auto generics = type.GenericParam();
        auto guard{ w.push_generic_params(generics) };

        if (!empty(generics))
        {
            w.write(R"(
template <)");

            separator s{ w };

            for (auto&& g : generics)
            {
                s();
                w.write("typename %", g.Name());
            }

            w.write(">");
        }

        auto format = R"(
void %()
{
    % o{ nullptr };
%%}
)";

        w.write(format,
            remove_tick(get_impl_name(type.TypeNamespace(), type.TypeName())),
            type,
            bind<write_consumer_method_list>(type),
            bind<write_consumer_interfaces>(get_interfaces(w, type)));
    }

    static void write_tests(cache const& c, task_group& group)
    {
        create_directories(settings.output_folder + "tests");
        writer all;

        for (auto&& [ns, members] : c.namespaces())
        {
            if (!has_projected_types(members) || !settings.projection_filter.includes(members))
            {
                continue;
            }

            all.write(R"(#include "winrt/%.h"
)",
ns);

            group.add([&ns = ns, &members = members]
            {
                writer w;

                w.write(R"(
#pragma comment(lib, "windowsapp")
#include "winrt/%.h"
using namespace winrt;

template <typename T>
T arg()
{
    throw hresult_not_implemented();
}
)",
ns);

                w.write_each<write_consumer_interface>(members.interfaces);
                w.write_each<write_consumer_class>(members.classes);

                auto w_path = settings.output_folder;
                w_path += "tests/";
                w_path += ns;
                w_path += ".cpp";

                w.flush_to_file(w_path);
            });
        }

        all.flush_to_file(settings.output_folder + "tests/all.h");
    }
}
