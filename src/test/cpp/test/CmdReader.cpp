#include "pch.h"
#include "winrt/Windows.Foundation.h"
#include "cmd_reader.h"
#include <fstream>

using namespace xlang::cmd;

TEST_CASE("CmdReader")
{
    const std::vector<option> options
    {
        { "input", 1 },
        { "reference", 0 },
        { "output", 0, 1 },
        { "component", 0, 1 },
        { "filter", 0 },
        { "name", 0, 1 },
        { "verbose", 0, 0 },
    };

    // input and output
    {
        const char* argv[] = { "progname", "-in", "example_file.in", "-out", "example_file.out" };
        const size_t argc = 5;
        reader args{ argc, argv, options };

        REQUIRE(args.exists("input"));
        REQUIRE(args.value("input") == "example_file.in");
        REQUIRE_FALSE(args.exists("reference"));
        REQUIRE(args.exists("output"));
        REQUIRE(args.value("output") == "example_file.out");
        REQUIRE_FALSE(args.exists("filter"));
        REQUIRE_FALSE(args.exists("name"));
        REQUIRE_FALSE(args.exists("verbose"));
    }

    // response file #1: filename no quotes
    {
        const char* argv[] = { "progname", "@respfile.txt" };
        const size_t argc = 2;
        std::ofstream resp_file;

        resp_file.open("respfile.txt");
        resp_file << "-in example_file.in -out example_file.out";
        resp_file.close();
        reader args{ argc, argv, options };
        std::remove("respfile.txt");

        REQUIRE(args.exists("input"));
        REQUIRE(args.value("input") == "example_file.in");
        REQUIRE_FALSE(args.exists("reference"));
        REQUIRE(args.exists("output"));
        REQUIRE(args.value("output") == "example_file.out");
        REQUIRE_FALSE(args.exists("filter"));
        REQUIRE_FALSE(args.exists("name"));
        REQUIRE_FALSE(args.exists("verbose"));
    }

    // response file #2: filename with quotes
    {
        const char* argv[] = { "progname", "@respfile.txt" };
        const size_t argc = 2;
        std::ofstream resp_file;

        resp_file.open("respfile.txt");
        resp_file << "-in \"example file.in\" -out \"example file.out\"";
        resp_file.close();
        reader args{ argc, argv, options };
        std::remove("respfile.txt");

        REQUIRE(args.exists("input"));
        REQUIRE(args.value("input") == "example file.in");
        REQUIRE_FALSE(args.exists("reference"));
        REQUIRE(args.exists("output"));
        REQUIRE(args.value("output") == "example file.out");
        REQUIRE_FALSE(args.exists("filter"));
        REQUIRE_FALSE(args.exists("name"));
        REQUIRE_FALSE(args.exists("verbose"));
    }

    // response file #3: filename with quote within name
    {
        const char* argv[] = { "progname", "@respfile.txt" };
        const size_t argc = 2;
        std::ofstream resp_file;

        resp_file.open("respfile.txt");
        resp_file << "-in example\\\"file.in -out example\\\"file.out";
        resp_file.close();
        reader args{ argc, argv, options };
        std::remove("respfile.txt");

        REQUIRE(args.exists("input"));
        REQUIRE(args.value("input") == "example\"file.in");
        REQUIRE_FALSE(args.exists("reference"));
        REQUIRE(args.exists("output"));
        REQUIRE(args.value("output") == "example\"file.out");
        REQUIRE_FALSE(args.exists("filter"));
        REQUIRE_FALSE(args.exists("name"));
        REQUIRE_FALSE(args.exists("verbose"));
    }

    // response file #4: really really long path
    {
        const char* argv[] = { "progname", "@respfile.txt" };
        const size_t argc = 2;
        std::ofstream resp_file;
        std::string file_name_in("C:\\");
        std::string file_name_out("C:\\");

        for (int i = 0; i < 500; i++) {
            file_name_in.append("dirname\\");
            file_name_out.append("dirname\\");
        }

        file_name_in.append("example_file.in");
        file_name_out.append("example_file.out");

        resp_file.open("respfile.txt");
        resp_file << "-in ";
        resp_file << file_name_in;
        resp_file << " -out ";
        resp_file << file_name_out;
        resp_file.close();
        reader args{ argc, argv, options };
        std::remove("respfile.txt");

        REQUIRE(args.exists("input"));
        REQUIRE(args.value("input") == file_name_in);
        REQUIRE_FALSE(args.exists("reference"));
        REQUIRE(args.exists("output"));
        REQUIRE(args.value("output") == file_name_out);
        REQUIRE_FALSE(args.exists("filter"));
        REQUIRE_FALSE(args.exists("name"));
        REQUIRE_FALSE(args.exists("verbose"));
    }
}
