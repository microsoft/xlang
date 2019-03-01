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
        w.current_namespace = name_space;

        write_license(w);
        write_jni_prolog(w, name_space);
        write_jni_stubs(w, name_space, members.classes);
        for (auto&& it : w.iterators)
        {
            write_jni_stub_iterator(w, it);
        }
        write_jni_unregisters(w, name_space);

        auto jni_path = w.write_temp("%cpp/%.cpp", settings.output_folder, name_space);
        w.flush_to_file(jni_path);

        return std::move(w.iterators);
    }

    static void create_java_file(writer w, java_type_name java_type)
    {
        std::experimental::filesystem::path java_path{
            w.write_temp("%java/%.java", settings.output_folder, java_type)
        };
        create_directories(java_path.parent_path());
        w.flush_to_file(java_path);
    }

    static void write_namespace(std::string_view const& name_space, cache::namespace_members const& members)
    {
        auto iterators = create_jni_stubs(name_space, members);
        for (auto&& it : iterators)
        {
            if (settings.filter.includes(name_space, it.name))
            {
                create_java_file(write_java_proxy_iterator(name_space, it), java_type_name{ it.name, name_space });
            }
        }
        for (auto&& type : members.interfaces)
        {
            if (settings.filter.includes(type))
            {
                create_java_file(write_java_interface(type), java_type_name{ type });
            }
        }
        for (auto&& type : members.classes)
        {
            if (settings.filter.includes(type))
            {
                create_java_file(write_java_proxy(type), java_type_name{ type });
            }
        }
        for (auto&& type : members.enums)
        {
            if (settings.filter.includes(type))
            {
                create_java_file(write_java_enum(type), java_type_name{ type });
            }
        }
        for (auto&& type : members.structs)
        {
            if (settings.filter.includes(type))
            {
                create_java_file(write_java_struct(type), java_type_name{ type });
            }
        }
        for (auto&& type : members.delegates)
        {
            if (settings.filter.includes(type))
            {
                create_java_file(write_java_delegate(type), java_type_name{ type });
            }
        }
    }
}
