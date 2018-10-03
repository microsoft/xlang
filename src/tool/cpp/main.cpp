#include "pch.h"
#include "version.h"
#include "strings.h"
#include "settings.h"
#include "type_writers.h"
#include "helpers.h"
#include "code_writers.h"
#include "file_writers.h"

namespace xlang
{
    settings_type settings;

    struct usage_exception {};

    void process_args(int const argc, char** argv)
    {
        std::vector<cmd::option> options
        {
            { "input", 1 },
            { "reference", 0 },
            { "output", 0, 1 },
            { "component", 0, 1 },
            { "filter", 0 },
            { "name", 0, 1 },
            { "verbose", 0, 0 },
            { "overwrite", 0, 0 },
            { "prefix", 0, 0 },
            { "license", 0, 0 },
            { "pch", 0, 1 },
            { "include", 0 },
            { "exclude", 0 },
            { "root", 0, 1 },
            { "base", 0, 0 }
        };

        cmd::reader args{ argc, argv, options };

        if (!args)
        {
            throw usage_exception{};
        }

        settings.verbose = args.exists("verbose");
        settings.root = args.value("root", "winrt");
        settings.input = args.files("input");
        settings.reference = args.files("reference");
        settings.component = args.exists("component");
        settings.base = args.exists("base");

        auto output_folder = absolute(args.value("output"));
        create_directories(output_folder / settings.root / "impl");
        output_folder += '/';
        settings.output_folder = output_folder.string();

        for (auto && include : args.values("include"))
        {
            settings.include.insert(include);
        }

        for (auto && include : args.values("filter"))
        {
            settings.include.insert(include);
        }

        for (auto && exclude : args.values("exclude"))
        {
            settings.exclude.insert(exclude);
        }

        if (settings.component)
        {
            settings.component_name = args.value("name");
            settings.component_pch = args.value("pch");
            auto component = args.value("component");

            if (!component.empty())
            {
                auto component_folder = absolute(component);
                create_directories(component_folder);
                component_folder += '/';
                settings.component_folder = component_folder.string();
            }
        }
    }

    auto get_files_to_cache()
    {
        std::vector<std::string> files;
        files.insert(files.end(), settings.input.begin(), settings.input.end());
        files.insert(files.end(), settings.reference.begin(), settings.reference.end());
        return files;
    }

    void supplement_includes(cache const& c)
    {
        if (settings.reference.empty() || !settings.include.empty())
        {
            return;
        }

        for (auto file : settings.input)
        {
            auto db = std::find_if(c.databases().begin(), c.databases().end(), [&](auto&& db)
            {
                return db.path() == file;
            });

            for (auto&& type : db->TypeDef)
            {
                if (!type.Flags().WindowsRuntime())
                {
                    continue;
                }

                settings.include.insert(std::string{ type.TypeNamespace() });
            }
        }
    }

    void run(int const argc, char** argv)
    {
        writer w;

        try
        {
            auto start = get_start_time();
            process_args(argc, argv);
            cache c{ get_files_to_cache() };
            c.remove_legacy_cppwinrt_foundation_types();
            supplement_includes(c);
            filter f{ settings.include, settings.exclude };

            if (settings.verbose)
            {
                for (auto&& file : settings.input)
                {
                    w.write("input: %\n", file);
                }

                for (auto&& file : settings.reference)
                {
                    w.write("reference: %\n", file);
                }

                w.write("output: %\n", settings.output_folder);
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
                if (f.empty() || settings.base)
                {
                    write_base_h();
                }

                if (settings.component)
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

                    if (!classes.empty())
                    {
                        write_component_g_cpp(classes);

                        for (auto&& type : classes)
                        {
                            write_component_g_h(type);
                            write_component_h(type);
                            write_component_cpp(type);
                        }
                    }
                }
            });

            group.get();

            if (settings.verbose)
            {
                w.write("time: %ms\n", get_elapsed_time(start));
            }
        }
        catch (usage_exception const&)
        {
            w.write("Usage...");
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
