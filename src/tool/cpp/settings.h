#pragma once

namespace xlang
{
    struct settings_type
    {
        std::set<std::string> input;
        std::set<std::string> reference;

        std::string output_folder;
        std::string root{ "winrt" };
        bool base{};

        bool component{};
        std::string component_folder;
        std::string component_name;
        std::string component_pch;
        bool component_prefix{};

        bool verbose{};
        bool overwrite{};
        bool uniform{};

        std::set<std::string> include;
        std::set<std::string> exclude;
    };

    extern settings_type settings;
}
