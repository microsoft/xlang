#pragma once

namespace xlang
{
    struct settings_type
    {
        std::set<std::string> input;

        std::experimental::filesystem::path output_folder;
        std::string module{ "pyrt" };
        bool verbose{};

        std::set<std::string> include;
        std::set<std::string> exclude;
    };

    extern settings_type settings;
}
