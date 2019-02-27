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

    //static void write_namespace_0_h(std::string_view const& name_space, cache::namespace_members const& members)
 //   {
 //       writer w;
 //       w.type_namespace = name_space;

 //       write_type_namespace(w, name_space);
 //       w.write_each<write_enum>(members.enums);
 //       w.write_each<write_forward>(members.interfaces);
 //       w.write_each<write_forward>(members.classes);
 //       w.write_each<write_forward>(members.structs);
 //       w.write_each<write_forward>(members.delegates);
 //       write_close_namespace(w);
 //       write_impl_namespace(w);
 //       w.write_each<write_enum_flag>(members.enums);
 //       w.write_each<write_category>(members.interfaces, "interface_category");
 //       w.write_each<write_category>(members.classes, "class_category");
 //       w.write_each<write_category>(members.enums, "enum_category");
 //       w.write_each<write_struct_category>(members.structs);
 //       w.write_each<write_category>(members.delegates, "delegate_category");
 //       w.write_each<write_name>(members.interfaces);
 //       w.write_each<write_name>(members.classes);
 //       w.write_each<write_name>(members.enums);
 //       w.write_each<write_name>(members.structs);
 //       w.write_each<write_name>(members.delegates);
 //       w.write_each<write_guid>(members.interfaces);
 //       w.write_each<write_fast_version>(members.classes);
 //       w.write_each<write_guid>(members.delegates);
 //       w.write_each<write_default_interface>(members.classes);
 //       w.write_each<write_interface_abi>(members.interfaces);
 //       w.write_each<write_delegate_abi>(members.delegates);
 //       w.write_each<write_fast_abi>(members.classes);
 //       w.write_each<write_consume>(members.interfaces);
 //       w.write_each<write_struct_abi>(members.structs);
 //       write_close_namespace(w);

 //       write_close_file_guard(w);
 //       w.swap();
 //       write_license(w);
 //       write_open_file_guard(w, name_space, '0');

 //       for (auto&& depends : w.depends)
 //       {
 //           write_type_namespace(w, depends.first);
 //           w.write_each<write_forward>(depends.second);
 //           write_close_namespace(w);
 //       }

 //       w.save_header('0');
 //   }

 //   static void write_namespace_1_h(std::string_view const& name_space, cache::namespace_members const& members)
 //   {
 //       writer w;
 //       w.type_namespace = name_space;

 //       write_type_namespace(w, name_space);
 //       w.write_each<write_interface>(members.interfaces);
 //       write_close_namespace(w);

 //       write_close_file_guard(w);
 //       w.swap();
 //       write_license(w);
 //       write_open_file_guard(w, name_space, '1');

 //       for (auto&& depends : w.depends)
 //       {
 //           w.write_depends(depends.first, '0');
 //       }

 //       w.write_depends(w.type_namespace, '0');
 //       w.save_header('1');
 //   }

 //   static void write_namespace_2_h(std::string_view const& name_space, cache::namespace_members const& members, cache const& c)
 //   {
 //       writer w;
 //       w.type_namespace = name_space;

 //       write_type_namespace(w, name_space);
 //       w.write_each<write_delegate>(members.delegates);
 //       write_structs(w, members.structs);
 //       w.write_each<write_class>(members.classes);
 //       w.write_each<write_interface_override>(members.classes);
 //       write_close_namespace(w);
 //       write_namespace_special(w, name_space, c);

 //       write_close_file_guard(w);
 //       w.swap();
 //       write_license(w);
 //       write_open_file_guard(w, name_space, '2');

 //       for (auto&& depends : w.depends)
 //       {
 //           w.write_depends(depends.first, '1');
 //       }

 //       w.write_depends(w.type_namespace, '1');
 //       w.save_header('2');
 //   }

 //   static void write_namespace_h(cache const& c, std::string_view const& name_space, cache::namespace_members const& members)
 //   {
 //       writer w;
 //       w.type_namespace = name_space;

 //       write_impl_namespace(w);
 //       w.write_each<write_consume_definitions>(members.interfaces);
 //       w.write_each<write_delegate_implementation>(members.delegates);
 //       w.write_each<write_produce>(members.interfaces);
 //       w.write_each<write_fast_produce>(members.classes);
 //       w.write_each<write_dispatch_overridable>(members.classes);
 //       write_close_namespace(w);
 //       write_type_namespace(w, name_space);
 //       w.write_each<write_class_definitions>(members.classes);

 //       w.write_each<write_delegate_definition>(members.delegates);
 //       w.write_each<write_interface_override_methods>(members.classes);
 //       w.write_each<write_class_override>(members.classes);
 //       write_close_namespace(w);
 //       write_std_namespace(w);
 //       w.write_each<write_std_hash>(members.interfaces);
 //       w.write_each<write_std_hash>(members.classes);
 //       write_close_namespace(w);

 //       write_close_file_guard(w);
 //       w.swap();
 //       write_license(w);
 //       write_open_file_guard(w, name_space);
 //       write_version_assert(w);

 //       for (auto&& depends : w.depends)
 //       {
 //           w.write_depends(depends.first, '2');
 //       }

 //       w.write_depends(w.type_namespace, '2');
 //       w.write_parent_depends(c);
 //       w.save_header();
 //   }

    static auto create_jni_stubs(std::string_view const& name_space, cache::namespace_members const& members)
    {
        writer w;
        w.type_namespace = name_space;

        //write_impl_namespace(w);
        //w.write_each<write_consume_definitions>(members.interfaces);
        //w.write_each<write_delegate_implementation>(members.delegates);
        //w.write_each<write_produce>(members.interfaces);
        //w.write_each<write_fast_produce>(members.classes);
        //w.write_each<write_dispatch_overridable>(members.classes);
        //write_close_namespace(w);
        //write_type_namespace(w, name_space);
        //w.write_each<write_class_definitions>(members.classes);

        //w.write_each<write_delegate_definition>(members.delegates);
        //w.write_each<write_interface_override_methods>(members.classes);
        //w.write_each<write_class_override>(members.classes);
        //write_close_namespace(w);
        //write_std_namespace(w);
        //w.write_each<write_std_hash>(members.interfaces);
        //w.write_each<write_std_hash>(members.classes);
        //write_close_namespace(w);

        write_license(w);
        write_jni_prolog(w, name_space);
        write_jni_stubs(w, name_space, members.classes);
        for (auto&& it : w.iterators)
        {
            write_jni_stub_iterator(w, it);
        }
        write_jni_unregisters(w, name_space, members.classes);

        //write_close_file_guard(w);
        //w.swap();
        
        //write_open_file_guard(w, name_space);
        //write_version_assert(w);

        //for (auto&& depends : w.depends)
        //{
        //    w.write_depends(depends.first, '2');
        //}

        //w.write_depends(w.type_namespace, '2');
        //w.write_parent_depends(c);

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
