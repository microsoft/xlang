#include "pch.h"
#include "version.h"
#include "strings.h"
#include "settings.h"
#include <experimental/filesystem>
#include "type_writers.h"
#include "helpers.h"
#include "code_writers.h"
#include "file_writers.h"

namespace xlang
{
    settings_type settings;

    struct usage_exception {};

    static constexpr cmd::option options[]
    {
        { "input", 0, cmd::option::no_max, "<spec>", "Metadata to include in projection" },
        { "reference", 0, cmd::option::no_max, "<spec>", "Metadata to reference from projection" },
        { "output", 0, 1, "<path>", "Location of generated cpp and java sources" },
        { "include", 0, cmd::option::no_max, "<prefix>", "One or more prefixes to include in input" },
        { "exclude", 0, cmd::option::no_max, "<prefix>", "One or more prefixes to exclude from input" },
        { "verbose", 0, 0, {}, "Show detailed progress information" },
        { "base", 0, 0, {}, "Generate jbase.h unconditionally" },
        { "lib", 0, 1, "<name>", "Set shared library name (defaults to package name)" },
        { "package", 0, 1, "<name>", "Set package base name (defaults to com.microsoft.)" },
        { "help", 0, cmd::option::no_max, {}, "Show detailed help" },
    };

    static void printUsage(writer& w)
    {
        static auto printColumns = [](writer& w, std::string_view const& col1, std::string_view const& col2)
        {
            w.write_printf("  %-20s%s\n", col1.data(), col2.data());
        };

        static auto printOption = [](writer& w, cmd::option const& opt)
        {
            if (opt.desc.empty())
            {
                return;
            }
            printColumns(w, w.write_temp("-% %", opt.name, opt.arg), opt.desc);
        };
            
        // todo: will 'javax' be confused with javax.* extension classes?
        auto format = R"(
Java/xlang Compiler v%

Usage: javax.exe [options...]

Options:

%  ^@<path>             Response file containing command line options

Where <spec> is one or more of:

  path                Path to winmd file or recursively scanned folder
  local               Local ^%WinDir^%\System32\WinMetadata folder
  sdk[+]              Current version of Windows SDK [with extensions]
  10.0.12345.0[+]     Specific version of Windows SDK [with extensions]
)";
        w.write(format, bind_each(printOption, options));
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
        settings.shared_lib = args.value("lib");
        settings.package_base = args.value("lib", "com.microsoft.");

        auto output_folder = canonical(args.value("output"));
        create_directories(output_folder / "cpp");
        create_directories(output_folder / "java" );
        output_folder += '/';
        settings.output_folder = output_folder.string();

        for (auto && include : args.values("include"))
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
            supplement_includes(c);
            settings.filter = { settings.include, settings.exclude };
            settings.base = settings.base || (settings.filter.empty());

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

                    write_namespace(ns, members);
                });
            }

            group.add([&]
            {
                if (settings.base)
                {
                    write_javart_h();
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
            printUsage(w);
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
