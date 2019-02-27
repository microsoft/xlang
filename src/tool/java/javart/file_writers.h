#pragma once

namespace xlang
{
    static void write_base_h()
    {
        writer w;
        write_license(w);
        w.write(strings::javart);
        w.flush_to_file(settings.output_folder + "cpp/javart.h");
    }

    static auto create_jni_stubs(std::string_view const& name_space, cache::namespace_members const& members)
    {
        writer w;
        w.type_namespace = name_space;

        write_license(w);
        write_jni_prolog(w, name_space);
        write_jni_stubs(w, name_space, members.classes);
        for (auto&& it : w.iterators)
        {
            write_jni_stub_iterator(w, it);
        }
        write_jni_unregisters(w, name_space, members.classes);

        auto jni_path = w.write_temp("%cpp/%.cpp", settings.output_folder, name_space);
        w.flush_to_file(jni_path);

        return std::move(w.iterators);
    }

    static void create_java_proxy(TypeDef const& type)
    {
        if (!settings.filter.includes(type))
        {
            return;
        }

        writer w;
        write_java_proxy(w, type);

        std::experimental::filesystem::path java_path{
            w.write_temp("%java/%.java", settings.output_folder, java_type_name{ type })
        };
        create_directories(java_path.parent_path());
        w.flush_to_file(java_path);
    }

    static void create_java_proxy_iteraror(std::string_view const& name_space, generic_iterator const& it)
    {
        writer w;
        write_java_proxy_iterator(w, name_space, it);

        std::experimental::filesystem::path java_path{
            w.write_temp("%java/%.java", settings.output_folder, java_type_name{ it.name, name_space })
        };
        create_directories(java_path.parent_path());
        w.flush_to_file(java_path);
    }

    static void write_namespace(std::string_view const& name_space, cache::namespace_members const& members)
    {
        auto iterators = create_jni_stubs(name_space, members);
        for (auto&& it : iterators)
        {
            create_java_proxy_iteraror(name_space, it);
        }
        for (auto&& type : members.classes)
        {
            create_java_proxy(type);
        }
    }
}
