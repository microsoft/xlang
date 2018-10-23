#include "pch.h"

#include "abi_writer.h"
#include "common.h"
#include "strings.h"

using namespace std::chrono;
using namespace std::experimental::filesystem;
using namespace std::literals;
using namespace xlang;
using namespace xlang::meta::reader;
using namespace xlang::text;
using namespace xlang::cmd;

auto output_directory(reader const& args)
{
    auto out{ absolute(args.value("output", "abi")) };
    create_directories(out);
    out += path::preferred_separator;
    return out.string();
}

void print_usage()
{
    puts("Usage...");
}

int main(int const argc, char** argv)
{
    console_writer w;

    try
    {
        auto const start = high_resolution_clock::now();

        std::vector<option> options
        {
            // name, min, max
            { "input", 1 },
            { "reference", 0 },
            { "output", 0, 1 },
            { "include", 0 },
            { "exclude", 0 },
            { "verbose", 0, 0 },
            { "ns-prefix", 0, 1 },
            { "enum-class", 0, 0 },
            { "lowercase-include-guard", 0, 0 },
        };

        reader args{ argc, argv, options };
        if (!args)
        {
            print_usage();
            return 0;
        }

        abi_configuration config;
        config.verbose = args.exists("verbose");
        config.output_directory = output_directory(args);
        config.enum_class = args.exists("enum-class");
        config.lowercase_include_guard = args.exists("lowercase-include-guard");

        if (args.exists("ns-prefix"))
        {
            auto const& values = args.values("ns-prefix");
            if (values.empty())
            {
                config.ns_prefix_state = ns_prefix::always;
            }
            else
            {
                auto const& value = values[0];
                if (value == "always")
                {
                    config.ns_prefix_state = ns_prefix::always;
                }
                else if (value == "never")
                {
                    config.ns_prefix_state = ns_prefix::never;
                }
                else if (value == "optional")
                {
                    config.ns_prefix_state = ns_prefix::optional;
                }
                else
                {
                    throw_invalid("'" + value + "' is not a valid argument for 'ns-prefix'");
                }
            }
        }
        else
        {
            config.ns_prefix_state = ns_prefix::never;
        }

        auto const& inputFiles = args.values("input");
        auto const& referenceFiles = args.values("reference");

        std::vector<std::string> filesToRead;
        filesToRead.reserve(inputFiles.size() + referenceFiles.size());
        filesToRead.insert(filesToRead.end(), inputFiles.begin(), inputFiles.end());
        filesToRead.insert(filesToRead.end(), referenceFiles.begin(), referenceFiles.end());

        cache c{ filesToRead };

        auto include = args.values("include");
        if (include.empty() && !referenceFiles.empty())
        {
            for (auto const& db : c.databases())
            {
                if (std::find_if(inputFiles.begin(), inputFiles.end(), [&](auto const& file)
                    {
                        return db.path() == file;
                    }) != inputFiles.end())
                {
                    for (auto const& type : db.TypeDef)
                    {
                        if (!type.Flags().WindowsRuntime())
                        {
                            continue;
                        }

                        include.push_back(std::string{ type.TypeNamespace() });
                    }
                }
            }
        }

        filter f{ include, args.values("exclude") };

        if (config.verbose)
        {
            for (auto const& in : inputFiles)
            {
                w.write("in: %\n", in);
            }

            for (auto const& ref : referenceFiles)
            {
                w.write("ref: %\n", ref);
            }

            w.write("out: %\n", config.output_directory);
        }

        w.flush_to_console();
        task_group group;

        for (auto const& [ns, members] : c.namespaces())
        {
            if (ns == collections_namespace)
            {
                // TODO: What to do with these?
                continue;
            }

            if (f.includes(members))
            {
                group.add([&]()
                {
                    write_abi_header(ns, c, members, config);
                });
            }
        }

        if (config.verbose)
        {
            w.write("time: %ms\n", duration_cast<milliseconds>((high_resolution_clock::now() - start)).count());
        }
    }
    catch (std::exception const& e)
    {
        w.write("%\n", e.what());
    }

    w.flush_to_console();
}
