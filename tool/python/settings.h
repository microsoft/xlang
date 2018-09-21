#pragma once

namespace xlang
{
    struct settings_type
    {
        std::set<std::string> input;

        std::string output_folder;
        std::string module{ "xlang" };
        bool verbose{};

        std::set<std::string> include;
        std::set<std::string> exclude;
    };

    extern settings_type settings;
}
