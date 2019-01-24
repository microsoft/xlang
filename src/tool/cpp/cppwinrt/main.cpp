#include "pch.h"
#include "version.h"
#include "strings.h"
#include "settings.h"
#include "type_writers.h"
#include "helpers.h"
#include "code_writers.h"
#include "component_writers.h"
#include "file_writers.h"
#include "type_writers.h"

namespace xlang
{
    settings_type settings;

    struct usage_exception {};

    static void process_args(int const argc, char** argv)
    {
        std::vector<cmd::option> options
        {
            { "input", 0 },
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
            { "base", 0, 0 },
            { "lib", 0, 1 },
            { "opt", 0, 0 },
            { "bracket", 0, 0 },
        };

        cmd::reader args{ argc, argv, options };

        if (!args)
        {
            throw usage_exception{};
        }

        settings.verbose = args.exists("verbose");
        settings.root = args.value("root", "winrt");

        settings.input = args.files("input", database::is_database);
        settings.reference = args.files("reference", database::is_database);

        settings.component = args.exists("component");
        settings.base = args.exists("base");

        auto output_folder = canonical(args.value("output"));
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
            settings.component_overwrite = args.exists("overwrite");
            settings.component_name = args.value("name");

            if (settings.component_name.empty() && settings.input.size() == 1)
            {
                settings.component_name = path(*settings.input.begin()).filename().replace_extension().string();
            }

            settings.component_pch = args.value("pch", "pch.h");
            settings.component_prefix = args.exists("prefix");
            settings.component_lib = args.value("lib", "winrt");
            settings.component_opt = args.exists("opt");

            if (settings.component_pch == ".")
            {
                settings.component_pch.clear();
            }

            auto component = args.value("component");

            if (!component.empty())
            {
                auto component_folder = canonical(component);
                create_directories(component_folder);
                component_folder += '/';
                settings.component_folder = component_folder.string();
            }
        }
    }

    static auto get_files_to_cache()
    {
        std::vector<std::string> files;
        files.insert(files.end(), settings.input.begin(), settings.input.end());
        files.insert(files.end(), settings.reference.begin(), settings.reference.end());
        return files;
    }

    static void supplement_includes(cache const& c)
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

    static bool has_projected_types(cache::namespace_members const& members)
    {
        return
            !members.interfaces.empty() ||
            !members.classes.empty() ||
            !members.enums.empty() ||
            !members.structs.empty() ||
            !members.delegates.empty();
    }

    static void run(int const argc, char** argv)
    {
        writer w;

        try
        {
            auto start = get_start_time();
            process_args(argc, argv);
            cache c{ get_files_to_cache() };
            c.remove_cppwinrt_foundation_types();
            supplement_includes(c);
            settings.filter = { settings.include, settings.exclude };
            settings.base = settings.base || !settings.component;

            if (settings.verbose)
            {
                w.write(" tool:  %\n", canonical(argv[0]).string());
                w.write(" ver:   %\n", XLANG_VERSION_STRING);

                for (auto&& file : settings.input)
                {
                    w.write(" in:    %\n", file);
                }

                for (auto&& file : settings.reference)
                {
                    w.write(" ref:   %\n", file);
                }

                w.write(" out:   %\n", settings.output_folder);

                if (!settings.component_folder.empty())
                {
                    w.write(" cout:  %\n", settings.component_folder);
                }
            }

            w.flush_to_console();
            task_group group;

            for (auto&&[ns, members] : c.namespaces())
            {
                group.add([&, &ns = ns, &members = members]
                {
                    if (!has_projected_types(members) || !settings.filter.includes(members))
                    {
                        return;
                    }

                    write_namespace_0_h(ns, members);
                    write_namespace_1_h(ns, members);
                    write_namespace_2_h(ns, members, c);
                    write_namespace_h(c, ns, members);
                });
            }

            group.add([&]
            {
                if (settings.base)
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
                            if (settings.filter.includes(type))
                            {
                                classes.push_back(type);
                            }
                        }
                    }

                    if (!classes.empty())
                    {
                        write_module_g_cpp(classes);

                        for (auto&& type : classes)
                        {
                            write_component_g_h(type);
                            write_component_g_cpp(type);
                            write_component_h(type);
                            write_component_cpp(type);
                        }
                    }
                }
            });

            group.get();

            if (settings.verbose)
            {
                w.write(" time:  %ms\n", get_elapsed_time(start));
            }
        }
        catch (usage_exception const&)
        {
            w.write("Usage...");
        }
        catch (std::exception const& e)
        {
            w.write(" error: %\n", e.what());
        }

        w.flush_to_console();
    }
}

int main(int const argc, char** argv)
{
    xlang::run(argc, argv);
}
