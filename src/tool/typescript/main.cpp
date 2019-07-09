#include "pch.h"
#include <time.h>
#include "settings.h"
#include "type_writers.h"
#include "helpers.h"
#include "code_writers.h"
#include "file_writers.h"
#include "type_writers.h"

namespace xlang
{
    settings_type settings;

    struct usage_exception {};

    static constexpr cmd::option options[]
    {
        { "input", 0, cmd::option::no_max, "<spec>", "Windows metadata to include in projection" },
        { "reference", 0, cmd::option::no_max, "<spec>", "Windows metadata to reference from projection" },
        { "output", 0, 1, "<path>", "Location of generated projection" },
        { "verbose", 0, 0, {}, "Show detailed progress information" },
        { "overwrite", 0, 0, {}, "Overwrite generated component files" },
        { "include", 0, cmd::option::no_max, "<prefix>", "One or more prefixes to include in input" },
        { "exclude", 0, cmd::option::no_max, "<prefix>", "One or more prefixes to exclude from input" },
        { "help", 0, cmd::option::no_max, {}, "Show detailed help with examples" },
        { "filter" }, // One or more prefixes to include in input (same as -include)
        { "license", 0, 0 }, // Generate license comment
    };

    static void print_usage(writer& w)
    {
        static auto printColumns = [](writer& w, std::string_view const& col1, std::string_view const& col2)
        {
            w.write_printf("  %-20s%s\n", col1.data(), col2.data());
        };

        static auto printOption = [](writer& w, cmd::option const& opt)
        {
            if(opt.desc.empty())
            {
                return;
            }
            printColumns(w, w.write_temp("-% %", opt.name, opt.arg), opt.desc);
        };

        auto format = R"(
Typescript .d.ts generator v%
Copyright (c) Microsoft Corporation. All rights reserved.

  tsrt.exe [options...]

Options:

%  ^@<path>             Response file containing command line options

Where <spec> is one or more of:

  path                Path to winmd file or recursively scanned folder
  local               Local ^%WinDir^%\System32\WinMetadata folder
  sdk[+]              Current version of Windows SDK [with extensions]
  10.0.12345.0[+]     Specific version of Windows SDK [with extensions]

)";
        w.write(format, XLANG_VERSION_STRING, bind_each(printOption, options));
    }

    static void process_args(int const argc, char** argv)
    {
        cmd::reader args{ argc, argv, options };

        if (!args || args.exists("help"))
        {
            throw usage_exception{};
        }

        settings.verbose = args.exists("verbose");

        settings.input = args.files("input", database::is_database);
        settings.reference = args.files("reference", database::is_database);

        settings.base = args.exists("base");

        settings.license = args.exists("license");

        auto output_folder = canonical(args.value("output"));
        create_directories(output_folder / "winrt/impl");
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
    }

    static auto get_files_to_cache()
    {
        std::vector<std::string> files;
        files.insert(files.end(), settings.input.begin(), settings.input.end());
        files.insert(files.end(), settings.reference.begin(), settings.reference.end());
        return files;
    }

    static void build_filters(cache const& c)
    {
        if (settings.reference.empty())
        {
            return;
        }

        std::set<std::string> include;

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

                std::string full_name{ type.TypeNamespace() };
                full_name += '.';
                full_name += type.TypeName();
                include.insert(full_name);
            }
        }

        settings.projection_filter = { include, {} };
    }

    static int run(int const argc, char** argv)
    {
        int result{};
        writer w;

        try
        {
            auto start = get_start_time();
            process_args(argc, argv);
            cache c{ get_files_to_cache() };
            build_filters(c);
            settings.base = settings.base || settings.projection_filter.empty();

            if (settings.verbose)
            {
                w.write(" tool:  %\n", canonical(argv[0]).string());
                w.write(" ver:   %\n", XLANG_VERSION_STRING);

                for (auto&& file : settings.input)
                {
                    w.write(" in:    %\n", file);
                }

                for (auto&& file : settings.include)
                {
                    w.write(" include: %\n", file);
                }

                for (auto&& file : settings.reference)
                {
                    w.write(" ref:   %\n", file);
                }

                w.write(" out:   %\n", settings.output_folder);
            }

            w.flush_to_console();
            task_group group;

            for (auto&&[ns, members] : c.namespaces())
            {
                group.add([&, &ns = ns, &members = members]
                {
                    if (!has_projected_types(members) || !settings.projection_filter.includes(members))
                    {
                        return;
                    }

                    write_namespace_d_ts(ns, members);
                });
            }

            group.get();

            if (settings.verbose)
            {
                w.write(" time:  %ms\n", get_elapsed_time(start));
            }
        }
        catch (usage_exception const&)
        {
            print_usage(w);
        }
        catch (std::exception const& e)
        {
            w.write(" error: %\n", e.what());
            result = 1;
        }

        w.flush_to_console();
        return result;
    }
}

int main(int const argc, char** argv)
{
    return xlang::run(argc, argv);
}
