#include "pch.h"

using namespace xlang;

TEST_CASE("cmd_reader")
{
    {
        // Simple key-value pair

        std::array<char const*, 3> args
        {
            "program.exe",
            "-key",
            "value"
        };

        std::vector<cmd::option> options
        {
            { "key", 1, 1 }
        };

        cmd::reader r{ args.size(), args.data(), options };

        REQUIRE(r);
        REQUIRE(r.exists("key"));
        REQUIRE(r.value("key") == "value");
        REQUIRE(r.values("key") == std::vector<std::string>{ "value" });
        REQUIRE(r.value("optional") == "");
        REQUIRE(r.value("optional", "default") == "default");
    }

    {
        // Invalid option

        std::array<char const*, 3> args
        {
            "program.exe",
            "invalid"
        };

        std::vector<cmd::option> options
        {
            { "key", 1, 1 }
        };

        try
        {
            cmd::reader r{ args.size(), args.data(), options };
            REQUIRE(false);
        }
        catch (std::invalid_argument const& e)
        {
            REQUIRE(e.what() == "Value 'invalid' is not supported"sv);
        }
    }

    {
        // Option with N values

        std::array<char const*, 6> args
        {
            "program.exe",
            "-key",
            "v1",
            "v2",
            "v3",
            "v4"
        };

        std::vector<cmd::option> options
        {
            { "key", 1 }
        };

        cmd::reader r{ args.size(), args.data(), options };

        REQUIRE(r);
        REQUIRE(r.exists("key"));
        REQUIRE(r.value("key") == "v1");
        REQUIRE(r.values("key") == std::vector<std::string>{ "v1", "v2", "v3", "v4" });
    }

    {
        // Matching options that start with arg

        std::array<char const*, 7> args
        {
            "program.exe",
            "-in", "input-value",
            "-o", "output-value",
            "-include", "include-value"
        };

        std::vector<cmd::option> options
        {
            { "input", 1, 1 },
            { "output", 1, 1 },
            { "include", 1, 1 }
        };

        cmd::reader r{ args.size(), args.data(), options };
        REQUIRE(r);
        REQUIRE(r.value("input") == "input-value");
        REQUIRE(r.value("output") == "output-value");
        REQUIRE(r.value("include") == "include-value");
    }
}
