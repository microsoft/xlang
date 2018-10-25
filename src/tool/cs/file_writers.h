#pragma once

namespace xlang
{
    inline auto write_namespace_cs(std::string_view const& ns, cache::namespace_members const& members)
    {
        writer w;
        w.current_namespace = ns;
        filter f{ settings.include, settings.exclude };
        auto filename = w.write_temp("%.cs", ns);

        w.write_license();
        write_type_namespace(w, ns);
        {
            writer::indent_guard g{ w };
            f.bind_each<write_enum>(members.enums)(w);
            f.bind_each<write_struct>(members.structs)(w);
            f.bind_each<write_interface>(members.interfaces)(w);
        }
        write_close_namespace(w);

        w.flush_to_file(settings.output_folder + filename);

        w.write("stamp");
        w.flush_to_file(settings.output_folder + "timestamp.txt");
        
        return std::move(w.needed_namespaces);
    }
}