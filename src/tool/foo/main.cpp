#include "pch.h"

namespace stdfs = std::experimental::filesystem;

struct writer : xlang::text::writer_base<writer>
{
    using xlang::text::writer_base<writer>::write;

    void write(std::size_t const value)
    {
        write(std::to_string(value));
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
    std::map<std::string_view, winrt_ns> sub_namespaces{};
    xlang::meta::reader::cache::namespace_members members{};
};

void write_indent(writer& w, int indent) noexcept
{
    for (int i = 0; i < indent; i++)
    {
        w.write("  ");
    }
}

void write_ns(writer& w, std::map<std::string_view, winrt_ns> const& ns, int indent) noexcept
{
    for (auto&& [name, _ns] : ns)
    {
        w.write("%% (%)\n", xlang::text::bind<write_indent>(indent), name, _ns.members.types.size());

        write_ns(w, _ns.sub_namespaces, indent + 1);
    }
}

xlang::meta::reader::cache::namespace_members const* find_ns(std::map<std::string_view, winrt_ns> const& namespaces, std::string_view const& ns) noexcept
{
    auto dot_pos = ns.find('.', 0);
    if (dot_pos == std::string_view::npos)
    {
        auto it = namespaces.find(ns);
        if (it == namespaces.end())
        {
            return nullptr;
        }
        else
        {
            return &(it->second.members);
        }
    }
    else
    {
        auto it = namespaces.find(ns.substr(0, dot_pos));
        if (it == namespaces.end())
        {
            return nullptr;
        }
        else
        {
            return find_ns(it->second.sub_namespaces, ns.substr(dot_pos + 1));
        }
    }
}

auto get_system_metadata()
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

    return std::move(files);
}

auto get_system_namespaces(xlang::meta::reader::cache const& c)
{
    std::map<std::string_view, winrt_ns> root_namespaces;

    for (auto&&[ns, members] : c.namespaces())
    {
        winrt_ns* curns{ nullptr };
        std::map<std::string_view, winrt_ns>* nsmap = &root_namespaces;

        for (auto&& s : get_dotted_name_segments(ns))
        {
            winrt_ns& q = (*nsmap)[s];
            curns = &q;
            nsmap = &(q.sub_namespaces);
        }

        curns->members = members;
    }

    return std::move(root_namespaces);
}

int main(int const /*argc*/, char** /*argv*/)
{
    writer w;

    auto start = get_start_time();

    xlang::meta::reader::cache c{ get_system_metadata() };
    auto namespaces = get_system_namespaces(c);

    auto elapsed = get_elapsed_time(start);

    write_ns(w, namespaces, 0);

    w.write("\n%ms\n", elapsed);
    w.write("  Windows.AI.MachineLearning %\n", find_ns(namespaces, "Windows.AI.MachineLearning") == nullptr ? "true" : "false");
    w.write("  Windows.AI.MachineLearningz %\n", find_ns(namespaces, "Windows.AI.MachineLearningz") == nullptr ? "true" : "false");

    w.flush_to_console();

    return 0;
}