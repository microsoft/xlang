#include "pch.h"
#include "version.h"
#include "strings.h"
#include "settings.h"
#include "type_writers.h"
#include "helpers.h"
#include "code_writers.h"
#include "file_writers.h"

namespace xlang::settings
{
    std::string output_folder;
    std::string component_folder;
    std::string root_folder{ "winrt" };
    std::string root_namespace{ "winrt" };
    std::string pch{ "pch.h" };
}

namespace xlang
{
    using namespace std::chrono;

    void print_usage()
    {
        puts("Usage...");
    }

    void prepare_usage(cmd::reader const& args)
    {
        settings::root_folder = args.value("root_folder", "winrt");
        settings::root_namespace = args.value("root_namespace", "winrt");

        auto path{ absolute(args.value("output")) };
        create_directories(path / settings::root_folder / "impl");
        path += path::preferred_separator;
        settings::output_folder = path.string();

        auto component = args.value("component");

        if (!component.empty())
        {
            path = absolute(component);
            create_directories(path);
            path += path::preferred_separator;
            settings::component_folder = path.string();
        }
    }

    void run(int const argc, char** argv)
    {
        writer w;

        try
        {
            auto start = get_start_time();

            std::vector<cmd::option> options
            {
                // name, min, max
                { "input", 1 },
                { "output", 0, 1 },
                { "include", 0 },
                { "exclude", 0 },
                { "verbose", 0, 0 },
                { "component", 0, 1 },
            };

            cmd::reader args{ argc, argv, options };

            if (!args)
            {
                print_usage();
                return;
            }

            prepare_usage(args);

            cache c{ args.values("input") };
            c.remove_legacy_cppwinrt_foundation_types();

            bool const verbose = args.exists("verbose");
            filter f(args.values("include"), args.values("exclude"));

            if (verbose)
            {
                for (auto&& db : c.databases())
                {
                    w.write("input: %\n", db.path());
                }

                w.write("output: %\n", settings::output_folder);
            }

            w.flush_to_console();
            task_group group;

            for (auto&&[ns, members] : c.namespaces())
            {
                group.add([&]
                {
                    if (!f.includes(members))
                    {
                        return;
                    }

                    write_namespace_0_h(ns, members);
                    write_namespace_1_h(ns, members);
                    write_namespace_2_h(ns, members);
                    write_namespace_h(c, ns, members);
                });
            }

            group.add([&]
            {
                // TODO: if ? don't write base.h
                write_base_h();

                if (args.exists("component"))
                {
                    std::vector<TypeDef> classes;

                    for (auto&&[ns, members] : c.namespaces())
                    {
                        for (auto&& type : members.classes)
                        {
                            if (f.includes(type))
                            {
                                classes.push_back(type);
                            }
                        }
                    }

                    write_component_g_cpp(classes);

                    for (auto&& type : classes)
                    {
                        write_component_g_h(type);
                        write_component_h(type);
                        write_component_cpp(type);
                    }
                }
            });

            group.get();

            if (verbose)
            {
                w.write("time: %ms\n", get_elapsed_time(start));
            }
        }
        catch (std::exception const& e)
        {
            w.write("%\n", e.what());
        }

        w.flush_to_console();
    }
}

int main(int const argc, char** argv)
{
    xlang::run(argc, argv);
}
