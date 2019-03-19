#pragma once

namespace xlang
{
    namespace stdfs = std::experimental::filesystem;

    inline void write_pybase_h(stdfs::path const& folder)
    {
        writer w;
        write_license_cpp(w);
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

        auto filename = w.write_temp("py.%.h", ns);

        auto segments = get_dotted_name_segments(ns);

        w.write("\nnamespace py::proj::%\n{", bind_list("::", segments));
        {
            writer::indent_guard g{ w };
            settings.filter.bind_each<write_pinterface_decl>(members.interfaces)(w);
        }
        w.write("}\n");

        w.write("\nnamespace py::impl::%\n{", bind_list("::", segments));
        {
            writer::indent_guard g{ w };
            settings.filter.bind_each<write_delegate_callable_wrapper>(members.delegates)(w);
            settings.filter.bind_each<write_pinterface_impl>(members.interfaces)(w);
        }
        w.write("}\n");

        w.write("\nnamespace py::wrapper::%\n{\n", bind_list("::", segments));
        {
            writer::indent_guard g{ w };
            settings.filter.bind_each<write_python_wrapper_alias>(members.classes)(w);
            settings.filter.bind_each<write_python_wrapper_alias>(members.interfaces)(w);
            settings.filter.bind_each<write_python_wrapper_alias>(members.structs)(w);
        }
        w.write("}\n");

        w.write("\nnamespace py\n{\n");
        {
            writer::indent_guard g{ w };
            settings.filter.bind_each<write_get_python_type_specialization>(members.classes)(w);
            settings.filter.bind_each<write_get_python_type_specialization>(members.interfaces)(w);
            settings.filter.bind_each<write_get_python_type_specialization>(members.structs)(w);
            settings.filter.bind_each<write_pinterface_type_mapper>(members.interfaces)(w);
            settings.filter.bind_each<write_delegate_type_mapper>(members.delegates)(w);
            settings.filter.bind_each<write_struct_converter_decl>(members.structs)(w);
        }
        w.write("}\n");

        w.swap();

        write_license_cpp(w);
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
        auto filename = w.write_temp("py.%.cpp", ns);

        write_license_cpp(w);
        w.write("#include \"py.%.h\"\n", ns);
        if (ns == "Windows.Foundation")
        {
            w.write(strings::custom_struct_convert);
        }
        settings.filter.bind_each<write_class>(members.classes)(w);
        settings.filter.bind_each<write_interface>(members.interfaces)(w);
        settings.filter.bind_each<write_struct>(members.structs)(w);
        write_namespace_initialization(w, ns, members);

        create_directories(folder);
        w.flush_to_file(folder / filename);
        return std::move(w.needed_namespaces);
    }

    inline void write_module_cpp(stdfs::path const& folder)
    {
        writer w;

        write_license_cpp(w);
        w.write(strings::module_methods, settings.module, settings.module, settings.module, settings.module);

        auto filename = w.write_temp("_%.cpp", settings.module);
        create_directories(folder);
        w.flush_to_file(folder / filename);
    }

    void write_namespace_cpp_filename(writer& w, std::string const& ns)
    {
        w.write("\"./%/src/py.%.cpp\"", settings.module, ns);
    }

    
    inline void write_setup_py(stdfs::path const& folder, std::vector<std::string> const& namespaces)
    {
        writer w;

        write_license_python(w);
        w.write(strings::setup, settings.module, settings.module, bind<write_setup_filenames>(namespaces));
        create_directories(folder);
        w.flush_to_file(folder / "setup.py");
    }

//    inline void write_cmake_lists_txt(stdfs::path const& folder, std::vector<std::string> const& namespaces)
//    {
//        writer w;
//
//        write_license_python(w);
//        w.write(R"(cmake_minimum_required(VERSION 3.12)
//
//add_compile_options(/std:c++17 /await)
//set(PYTHON_PATH "C:/Users/hpierson/AppData/Local/Programs/Python/Python37-32/")
//
//message(${PYTHON_PATH})
//message(${UNSET_PYTHON_PATH})
//
//link_directories("${PYTHON_PATH}/libs")
//
//)");
//        w.write("set(sources\n    %\n    \"./%/src/_%.cpp\")\n\n",
//            bind_list<write_namespace_cpp_filename>("\n    ", namespaces),
//            settings.module, settings.module);
//
//		auto format = R"(
//project(%)
//add_library(% SHARED ${sources})
//target_include_directories(% PUBLIC "${PYTHON_PATH}/include")    
//target_link_libraries(% windowsapp)
//set_target_properties(% PROPERTIES SUFFIX ".pyd")
//)";
//		w.write(format, settings.module, settings.module, settings.module, settings.module, settings.module);
//        create_directories(folder);
//        w.flush_to_file(folder / "CMakeLists.txt");
//    }

    inline void write_package_dunder_init_py(stdfs::path const& folder)
    {
        writer w;

        write_license_python(w);
        w.write(strings::package_init, settings.module, settings.module, settings.module, settings.module);

        create_directories(folder);
        w.flush_to_file(folder / "__init__.py");
    }

    inline void write_namespace_dunder_init_py(stdfs::path const& folder, std::string_view const& module_name, std::set<std::string> const& needed_namespaces, std::string_view const& ns, cache::namespace_members const& members)
    {
        writer w;
        w.current_namespace = ns;
        
        write_license_python(w);

        w.write("import typing, %\n", module_name);

        if (settings.filter.includes(members.enums))
        {
            w.write("import enum\n");
        }

        w.write("\n_ns_module = %._import_ns_module(\"%\")\n\n", module_name, ns);

        if (!needed_namespaces.empty())
        {
            for (std::string needed_ns : needed_namespaces)
            {
                std::transform(needed_ns.begin(), needed_ns.end(), needed_ns.begin(), [](char c) {return static_cast<char>(::tolower(c)); });
                w.write("import %.%\n", module_name, needed_ns);
            }
        }

        w.write("\n");

        settings.filter.bind_each<write_python_enum>(members.enums)(w);
        settings.filter.bind_each<write_python_class>(members.classes)(w);
        //settings.filter.bind_each<write_import_type>(members.interfaces)(w);
        //settings.filter.bind_each<write_import_type>(members.structs)(w);

        create_directories(folder);
        w.flush_to_file(folder / "__init__.py");
    }
}