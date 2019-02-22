#pragma once

namespace rgm
{
    struct settings_type
    {
        std::set<std::string> input;
        std::set<std::string> reference;
    };

    extern settings_type settings;
}
