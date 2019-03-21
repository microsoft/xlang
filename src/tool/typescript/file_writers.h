#pragma once

namespace xlang
{
    static void write_namespace_d_ts(std::string_view const& ns, cache::namespace_members const& members)
    {
        writer w;
        w.type_namespace = ns;

        write_preamble(w);
        write_type_namespace(w, ns);

        w.write_each<write_enum>(members.enums);
        w.write_each<write_interface>(members.interfaces);

        write_close_namespace(w);

        w.save_definition();
    }
}
