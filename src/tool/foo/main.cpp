#include "pch.h"

namespace stdfs = std::experimental::filesystem;

struct writer : xlang::text::writer_base<writer>
{
    using xlang::text::writer_base<writer>::write;
};

auto get_start_time()
{
    return std::chrono::high_resolution_clock::now();
}

auto get_elapsed_time(std::chrono::time_point<std::chrono::high_resolution_clock> const& start)
{
    return std::chrono::duration_cast<std::chrono::duration<int64_t, std::milli>>(std::chrono::high_resolution_clock::now() - start).count();
}

int main(int const /*argc*/, char** /*argv*/)
{
    stdfs::path winmd{ "C:\\Windows\\System32\\WinMetadata" }; 

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
    }

    auto elapsed = get_elapsed_time(start);

    w.write("\n%ms\n", elapsed);
    w.flush_to_console();

    return 0;
}