#include "pch.h"

namespace coolrt
{
    using namespace std::literals;
    using namespace std::experimental::filesystem;
    using namespace xlang;
    using namespace xlang::meta::reader;
    using namespace xlang::text;

    inline auto get_start_time()
    {
        return std::chrono::high_resolution_clock::now();
    }

    inline auto get_elapsed_time(std::chrono::time_point<std::chrono::high_resolution_clock> const& start)
    {
        return std::chrono::duration_cast<std::chrono::duration<int64_t, std::milli>>(std::chrono::high_resolution_clock::now() - start).count();
    }

    struct writer : xlang::text::writer_base<writer>
    {
    };

    struct settings_type
    {
        std::set<std::string> input;

        std::experimental::filesystem::path output_folder;
        std::string module{ "pyrt" };
        bool verbose{};

        std::set<std::string> include;
        std::set<std::string> exclude;
        xlang::meta::reader::filter filter;
    };

    settings_type settings;

    struct usage_exception {};

    static constexpr cmd::option options[]
    {
        { "input", 0, cmd::option::no_max, "<spec>", "Windows metadata to include in projection" },
        { "output", 0, 1, "<path>", "Location of generated projection" },
        { "include", 0, cmd::option::no_max, "<prefix>", "One or more prefixes to include in projection" },
        { "exclude", 0, cmd::option::no_max, "<prefix>", "One or more prefixes to exclude from projection" },
        { "verbose", 0, 0, {}, "Show detailed progress information" },
        { "module", 0, 1, "<name>", "Name of generated projection. Defaults to winrt."},
        { "help", 0, cmd::option::no_max, {}, "Show detailed help" },
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
Cool/WinRT v%
Copyright (c) Microsoft Corporation. All rights reserved.

  coolrt.exe [options...]

Options:

%  ^@<path>             Response file containing command line options

Where <spec> is one or more of:

  path                Path to winmd file or recursively scanned folder
  local               Local ^%WinDir^%\System32\WinMetadata folder
  sdk[+]              Current version of Windows SDK [with extensions]
  10.0.12345.0[+]     Specific version of Windows SDK [with extensions]
)";
        w.write(format, "0.0.0", bind_each(printOption, options));
    }

    void process_args(int const argc, char** argv)
    {
        cmd::reader args{ argc, argv, options };

        if (!args || args.exists("help"))
        {
            throw usage_exception{};
        }

        settings.verbose = args.exists("verbose");
        settings.module = args.value("module", "winrt");
        settings.input = args.files("input", database::is_database);

        for (auto && include : args.values("include"))
        {
            settings.include.insert(include);
        }

        for (auto && exclude : args.values("exclude"))
        {
            settings.exclude.insert(exclude);
        }

        settings.output_folder = absolute(args.value("output", "output"));
        create_directories(settings.output_folder);
    }

    auto get_files_to_cache()
    {
        std::vector<std::string> files;
        files.insert(files.end(), settings.input.begin(), settings.input.end());
        return files;
    }

    bool has_projected_types(cache::namespace_members const& members)
    {
        return
            !members.interfaces.empty() ||
            !members.classes.empty() ||
            !members.enums.empty() ||
            !members.structs.empty() ||
            !members.delegates.empty();
    }

    int run(int const argc, char** argv)
    {
        int result{};
        writer w;

        try
        {
            auto start = get_start_time();
            process_args(argc, argv);
            cache c{ get_files_to_cache() };
            settings.filter = { settings.include, settings.exclude };

            if (settings.verbose)
            {
                for (auto&& file : settings.input)
                {
                    w.write("input: %\n", file);
                }

                w.write("output: %\n", settings.output_folder.string());
            }

            w.flush_to_console();

            task_group group;

            for (auto&&[ns, members] : c.namespaces())
            {
                if (!has_projected_types(members) || !settings.filter.includes(members))
                {
                    continue;
                }

                w.write("%\n", ns);
            }

            group.get();

            if (settings.verbose)
            {
                w.write("time: %ms\n", get_elapsed_time(start));
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
    return coolrt::run(argc, argv);
}