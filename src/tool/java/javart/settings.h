#pragma once

namespace xlang
{
    struct settings_type
    {
        std::set<std::string> input;
        std::set<std::string> reference;

        bool base{};
        std::string output_folder;
        std::string shared_lib;
        std::string package_base;

        bool verbose{};

        std::set<std::string> include;
        std::set<std::string> exclude;
        meta::reader::filter filter;
    };

    extern settings_type settings;
}
