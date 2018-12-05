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

bool is_exclusive_to(xlang::meta::reader::TypeDef const& type)
{
    return xlang::meta::reader::get_category(type) == xlang::meta::reader::category::interface_type && xlang::meta::reader::get_attribute(type, "Windows.Foundation.Metadata", "ExclusiveToAttribute");
}

struct winrt_ns
{
    std::string namespace_name;
    std::vector<winrt_ns> sub_namespaces{};
    xlang::meta::reader::cache::namespace_members members{};
};

xlang::meta::reader::cache::namespace_members const* find_ns(std::vector<winrt_ns> const& namespaces, std::string_view const& ns)
{
    auto dot_pos = ns.find('.', 0);
    if (dot_pos == std::string_view::npos)
    {
        auto it = std::find_if(namespaces.begin(), namespaces.end(), [ns](auto const& wns) { return wns.namespace_name == ns; });
        if (it == namespaces.end())
        {
            return nullptr;
        }
        else
        {
            return &(it->members);
        }
    }
    else
    {
        auto it = std::find_if(namespaces.begin(), namespaces.end(), [ns = ns.substr(0, dot_pos)](auto const& wns) { return wns.namespace_name == ns; });
        if (it == namespaces.end())
        {
            return nullptr;
        }
        else
        {
            return find_ns(it->sub_namespaces, ns.substr(dot_pos + 1));
        }
    }
}

void write_ns(writer& w, std::vector<winrt_ns> const& ns, int indent)
{
    for (auto&& sub : ns)
    {
        for (int i = 0; i < indent; i++)
        {
            w.write("  ");
        }
        w.write("%\n", sub.namespace_name);

        for (auto&&[n, td] : sub.members.types)
        {
            if (is_exclusive_to(td)) { continue; }

            for (int i = 0; i < indent; i++)
            {
                w.write("  ");
            }

            w.write("**%\n", n);
            break;
        }

        write_ns(w, sub.sub_namespaces, indent + 1);
    }
}

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
    std::vector<winrt_ns> root_namespaces;

    for (auto&&[ns, members] : c.namespaces())
    {
        std::vector<winrt_ns>::iterator cur;
        auto* nsmap = &root_namespaces;

        for (auto&& s : get_dotted_name_segments(ns))
        {
            cur = std::find_if(nsmap->begin(), nsmap->end(), [s](winrt_ns const& n) { return n.namespace_name == s; });
            if (cur == nsmap->end())
            {
                cur = nsmap->insert(nsmap->end(), winrt_ns{ std::string{s} });
            }

            nsmap = &(cur->sub_namespaces);
        }

        cur->members = members;
    }

    auto elapsed = get_elapsed_time(start);

    auto q = find_ns(root_namespaces, "Windows.AI.MachineLearning");

    write_ns(w, root_namespaces, 0);
    w.write("\n%ms\n", elapsed);
    w.flush_to_console();

    return 0;
}