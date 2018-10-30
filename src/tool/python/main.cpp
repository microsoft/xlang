#include "pch.h"
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
            { "output", 0, 1 },
            { "include", 0 },
            { "exclude", 0 },
            { "verbose", 0, 0 },
            { "module", 0, 1 },
        };

        cmd::reader args{ argc, argv, options };

        if (!args)
        {
            throw usage_exception{};
        }

        settings.verbose = args.exists("verbose");
        settings.module = args.value("module", "pyrt");
        settings.input = args.files("input");

        for (auto && include : args.values("include"))
        {
            settings.include.insert(include);
        }

        for (auto && exclude : args.values("exclude"))
        {
            settings.exclude.insert(exclude);
        }

        auto output_folder = absolute(args.value("output", "output"));
        create_directories(output_folder);
        output_folder += '/';
        settings.output_folder = output_folder.string();
    }

    auto get_files_to_cache()
    {
        std::vector<std::string> files;
        files.insert(files.end(), settings.input.begin(), settings.input.end());
        return files;
    }

    int run(int const argc, char** argv)
    {
        writer wc;

        try
        {
            auto start = get_start_time();
            process_args(argc, argv);
            cache c{ get_files_to_cache() };
            filter f{ settings.include, settings.exclude };

            if (settings.verbose)
            {
                for (auto&& file : settings.input)
                {
                    wc.write("input: %\n", file);
                }

                wc.write("output: %\n", settings.output_folder);
            }

            wc.flush_to_console();

            std::vector<std::string> generated_namespaces{};
            task_group group;

            group.add([&]
            {
                write_pybase_h();
            });

            for (auto&& ns : c.namespaces())
            {
                if (!f.includes(ns.second))
                {
                    continue;
                }

                std::string fqns{ ns.first };
                auto h_filename = "py." + fqns + ".h";

                generated_namespaces.emplace_back(ns.first);

                group.add([&]
                {
                    auto namespaces = write_namespace_cpp(ns.first, ns.second);
                    write_namespace_h(ns.first, namespaces, ns.second);
                });
            }

            group.get();

            auto native_module = "_" + settings.module;
            write_module_cpp(native_module, generated_namespaces);
            write_setup_py(settings.module, native_module, generated_namespaces);

            if (settings.verbose)
            {
                wc.write("time: %ms\n", get_elapsed_time(start));
            }
        }
        catch (std::exception const& e)
        {
            wc.write("%\n", e.what());
            wc.flush_to_console();
            getchar();
            return -1;
        }

        wc.flush_to_console();
        return 0;
    }
}

int main(int const argc, char** argv)
{
    return xlang::run(argc, argv);
}