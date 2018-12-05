#include "pch.h"

namespace stdfs = std::experimental::filesystem;

struct writer : xlang::text::writer_base<writer>
{
    using xlang::text::writer_base<writer>::write;
};

struct separator
{
    writer& w;
    std::string_view _separator{ ", " };
    bool first{ true };

    void operator()()
    {
        if (first)
        {
            first = false;
        }
        else
        {
            w.write(_separator);
        }
    }
};

auto get_start_time()
{
    return std::chrono::high_resolution_clock::now();
}

auto get_elapsed_time(std::chrono::time_point<std::chrono::high_resolution_clock> const& start)
{
    return std::chrono::duration_cast<std::chrono::duration<int64_t, std::milli>>(std::chrono::high_resolution_clock::now() - start).count();
}

auto get_dotted_name_segments(std::string_view ns)
{
    std::vector<std::string_view> segments;
    size_t pos = 0;

    while (true)
    {
        auto new_pos = ns.find('.', pos);

        if (new_pos == std::string_view::npos)
        {
            segments.push_back(ns.substr(pos));
            return std::move(segments);
        }

        segments.push_back(ns.substr(pos, new_pos - pos));
        pos = new_pos + 1;
    };
};

int main(int const /*argc*/, char** /*argv*/)
{
#ifdef _WIN64
    auto sys32 = "c:\\Windows\\System32";
#else
    auto sys32 = "c:\\Windows\\Sysnative";
#endif

    auto winmd = stdfs::path{ sys32 } / "WinMetadata";

    std::vector<std::string> files;
    for (auto& p : stdfs::directory_iterator(winmd))
    {
        files.push_back(p.path().string());
    }

    writer w;

    auto start = get_start_time();

    xlang::meta::reader::cache c{ files };

    for (auto&&[ns, members] : c.namespaces())
    {
        w.write("%\n", ns);
        auto segments = get_dotted_name_segments(ns);

        for (decltype(segments.size()) i = 0; i < segments.size(); i++)
        {
            separator s{ w, "." };
            w.write("\t");

            for (decltype(i) j = 0; j <= i; j++)
            {
                s();
                w.write(segments[j]);
            }
            w.write("\n");
        }
    }

    auto elapsed = get_elapsed_time(start);

    w.write("\n%ms\n", elapsed);
    w.flush_to_console();

    return 0;
}