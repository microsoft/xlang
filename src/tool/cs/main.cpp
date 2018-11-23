#include "pch.h"
#include "version.h"
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
        writer w;

        try
        {
            auto start = get_start_time();
            process_args(argc, argv);
            cache c{ get_files_to_cache() };
            filter f{ settings.include, settings.exclude };

            if (settings.verbose)
            {
                w.write(" tool:  % (C#/WinRT v%)\n", canonical(argv[0]).string(), XLANG_VERSION_STRING);

                for (auto&& file : settings.input)
                {
                    w.write(" in:    %\n", file);
                }

                w.write(" out:   %\n", settings.output_folder);
            }

            w.flush_to_console();

            task_group group;

            for (auto&& ns : c.namespaces())
            {
                if (!f.includes(ns.second))
                {
                    continue;
                }

                group.add([&]
                {
                    write_namespace_cs(ns.first, ns.second);
                });
            }

            group.get();

            if (settings.verbose)
            {
                w.write(" time:  %ms\n", get_elapsed_time(start));
            }
        }
        catch (std::exception const& e)
        {
            w.write(" error: %\n", e.what());
            return -1;
        }

        w.flush_to_console();
        return 0;
    }
}

int main(int const argc, char** argv)
{
    return xlang::run(argc, argv);
}