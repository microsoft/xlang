#pragma once

namespace xlang
{
    static void write_tests(cache const& c)
    {
        create_directories(settings.output_folder + "tests");
        writer all;

        for (auto&& [ns, members] : c.namespaces())
        {
            if (!has_projected_types(members) || !settings.projection_filter.includes(members))
            {
                continue;
            }

            all.write(R"(#include "winrt/%.h"
)",
                ns);

            writer w;

            w.write(R"(#include "winrt/%.h"
)",
                ns);

            auto w_path = settings.output_folder;
            w_path += "tests/";
            w_path += ns;
            w_path += ".cpp";

            w.flush_to_file(w_path);
        }

        all.flush_to_file(settings.output_folder + "tests/all.h");
    }
}
