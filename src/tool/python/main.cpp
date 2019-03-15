#include "pch.h"
#include "strings.h"
#include "settings.h"
#include "type_writers.h"
#include "helpers.h"
#include "code_writers.h"
#include "python_code_writers.h"
#include "file_writers.h"

namespace xlang
{
    settings_type settings;

    struct usage_exception {};

    void process_args(int const argc, char** argv)
    {
        static constexpr cmd::option options[]
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
        settings.module = args.value("module", "winrt");
        settings.input = args.files("input");

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

    int run(int const argc, char** argv)
    {
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

            auto module_dir = settings.output_folder / settings.module;
            auto src_dir = module_dir / "src";

            group.add([&]
            {
                write_pybase_h(src_dir);
                write_package_dunder_init_py(module_dir);
                write_module_cpp(src_dir);
            });

            std::vector<std::string> generated_namespaces{};

            for (auto&&[ns, members] : c.namespaces())
            {
                if (!settings.filter.includes(members))
                {
                    continue;
                }

                auto ns_dir = module_dir;
                
                auto append_dir = [&ns_dir](std::string_view const& ns_segment)
                {
                    std::string segment{ ns_segment };
                    std::transform(segment.begin(), segment.end(), segment.begin(), [](char c) {return static_cast<char>(::tolower(c)); });
                    ns_dir /= segment;
                };
                
                size_t pos{};
                while (true)
                {
                    auto new_pos = ns.find('.', pos);
                    if (new_pos == std::string_view::npos)
                    { 
                        append_dir(ns.substr(pos));
                        break;
                    }

                    append_dir(ns.substr(pos, new_pos - pos));
                    pos = new_pos + 1;
                } 

                generated_namespaces.emplace_back(ns);

                group.add([&, &ns = ns, &members = members]
                {
                    auto namespaces = write_namespace_cpp(src_dir, ns, members);
                    write_namespace_h(src_dir, ns, namespaces, members);
                    write_namespace_dunder_init_py(ns_dir, settings.module, namespaces, ns, members);
                });
            }

            group.get();

            write_setup_py(settings.output_folder, generated_namespaces);
            write_cmake_lists_txt(settings.output_folder, generated_namespaces);

            if (settings.verbose)
            {
                w.write("time: %ms\n", get_elapsed_time(start));
            }
        }
        catch (std::exception const& e)
        {
            w.write("%\n", e.what());
            w.flush_to_console();
            getchar();
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