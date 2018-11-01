#pragma once

namespace xlang
{
    namespace stdfs = std::experimental::filesystem;

    inline void write_pybase_h(stdfs::path const& folder)
    {
        writer w;
        w.write(strings::pybase);
        create_directories(folder);
        w.flush_to_file(folder / "pybase.h");
    }

    inline void write_namespace_h(stdfs::path const& folder, std::string_view const& ns, std::set<std::string> const& needed_namespaces, cache::namespace_members const& members)
    {
        writer w;
        w.current_namespace = ns;

        for (auto&& needed_ns : needed_namespaces)
        {
            w.needed_namespaces.insert(needed_ns);
        }

        xlang::filter f{ settings.include, settings.exclude };
        auto filename = w.write_temp("py.%.h", ns);

        f.bind_each<write_delegate>(members.delegates)(w);
        f.bind_each<write_pinterface_decl>(members.interfaces)(w);
        f.bind_each<write_pinterface_impl>(members.interfaces)(w);

        w.write("\nnamespace py\n{\n");
        {
            writer::indent_guard g{ w };
            f.bind_each<write_winrt_type_specialization>(members.classes)(w);
            f.bind_each<write_winrt_type_specialization>(members.interfaces)(w);
            f.bind_each<write_winrt_type_specialization>(members.structs)(w);
            f.bind_each<write_struct_converter_decl>(members.structs)(w);
            f.bind_each<write_pinterface_type_mapper>(members.interfaces)(w);
            f.bind_each<write_delegate_type_mapper>(members.delegates)(w);
        }
        w.write("}\n");

        w.swap();

        w.write_license();
        {
            auto format = R"(#pragma once

#include "pybase.h"
)";
            w.write(format);
        }

        w.write_each<write_include>(w.needed_namespaces);

        {
            auto format = R"(
#include <winrt/%.h>

)";
            w.write(format, ns);
        }

        create_directories(folder);
        w.flush_to_file(folder / filename);
    }

    inline auto write_namespace_cpp(stdfs::path const& folder, std::string_view const& ns, cache::namespace_members const& members)
    {
        writer w;
        w.current_namespace = ns;
        filter f{ settings.include, settings.exclude };
        auto filename = w.write_temp("py.%.cpp", ns);

        w.write_license();
        write_include(w, ns);
        f.bind_each<write_class>(members.classes)(w);
        f.bind_each<write_interface>(members.interfaces)(w);
        f.bind_each<write_struct>(members.structs)(w);
        write_namespace_init(w, f, ns, members);

        create_directories(folder);
        w.flush_to_file(folder / filename);
        return std::move(w.needed_namespaces);
    }

    inline void write_module_cpp(stdfs::path const& folder, std::string_view const& module_name, std::vector<std::string> const& namespaces)
    {
        writer w;

        w.write_license();
        write_python_namespace_includes(w, namespaces);
        w.write(strings::module_methods);
        write_module_exec(w);
        write_module_slots(w);
        write_module_def(w, module_name);
        write_module_init_func(w, module_name);

        auto filename = w.write_temp("%.cpp", module_name);
        create_directories(folder);
        w.flush_to_file(folder / filename);
    }

    inline void write_setup_py(stdfs::path const& folder, std::string_view const& module_name, std::string_view const& native_module_name, std::vector<std::string> const& namespaces)
    {
        writer w;

        w.write(strings::setup, module_name, native_module_name, bind<write_setup_filenames>(native_module_name, namespaces));
        create_directories(folder);
        w.flush_to_file(folder / "setup.py");
    }

    inline void write_package_init(stdfs::path const& folder, std::string_view const& module_name)
    {
        writer w;

        w.write(strings::package_init, module_name, module_name, module_name, module_name);
        create_directories(folder);
        w.flush_to_file(folder / "__init__.py");
    }

    inline void write_namespace_init(stdfs::path const& folder, std::string_view const& module_name, std::set<std::string> const& needed_namespaces, std::string_view const& ns, cache::namespace_members const& members)
    {
        writer w;

        w.write(strings::ns_init, module_name, ns);

        if (!needed_namespaces.empty())
        {
            for (auto&& needed_ns : needed_namespaces)
            {
                auto ns = needed_ns;
                std::transform(ns.begin(), ns.end(), ns.begin(), [](char c) {return static_cast<char>(::tolower(c)); });
                auto format = R"(try:
    import %.%
except:
    pass
)";
                w.write_indented(format, module_name, ns);
            }
            w.write("\n");
        }

        xlang::filter f{ settings.include, settings.exclude };
        f.bind_each<write_import_type>(members.classes)(w);
        f.bind_each<write_import_type>(members.interfaces)(w);
        f.bind_each<write_import_type>(members.structs)(w);

        create_directories(folder);
        w.flush_to_file(folder / "__init__.py");
    }
}