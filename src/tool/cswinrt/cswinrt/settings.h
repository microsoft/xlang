#pragma once

namespace cswinrt
{
    struct settings_type
    {
        std::set<std::string> input;
        std::experimental::filesystem::path output_folder;
        bool verbose{};
        std::set<std::string> include;
        std::set<std::string> exclude;
        xlang::meta::reader::filter filter;
    };

    extern settings_type settings;
}
