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

int %(PyObject* module);
)";
            w.write(format, ns, bind<write_ns_init_function_name>(ns));
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
        write_module_exec(w, namespaces);
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
}