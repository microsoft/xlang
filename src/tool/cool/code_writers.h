#pragma once

namespace coolrt
{
    using namespace xlang::meta::reader;

    void write_members(writer& w, TypeDef const& type)
    {
        for (auto&& method : type.MethodList())
        {
            if (!is_constructor(method))
            {
                continue;
            }

            w.write("// ctor @\n", type.TypeName());
        }

        for (auto&& method : type.MethodList())
        {
            if (is_special(method))
            {
                continue;
            }

            w.write("// %\n", method.Name());
        }

        for (auto&& prop : type.PropertyList())
        {
            w.write("// prop %\n", prop.Name());
        }

        for (auto&& evt : type.EventList())
        {
            w.write("// prop %\n", evt.Name());
        }
    }

    void write_class(writer& w, TypeDef const& type)
    {
        w.write("public class @\n{\n", type.TypeName());
        {
            writer::indent_guard g{ w };
            write_members(w, type);
        }
        w.write("}\n");
    }

    void write_delegate(writer& w, TypeDef const& type)
    {
        method_signature signature{ get_delegate_invoke(type) };

        w.write("public delegate void @();\n", type.TypeName());
    }

    void write_enum(writer& w, TypeDef const& type)
    {
        if (is_flags_enum(type))
        {
            w.write("[System.FlagsAttribute]");
        }

        w.write("public enum @ : %\n{\n", type.TypeName(), is_flags_enum(type) ? "int" : "uint");
        {
            writer::indent_guard g{ w };

            for (auto&& field : type.FieldList())
            {
                if (auto constant = field.Constant())
                {
                    w.write("% = %,\n", field.Name(), *constant);
                }
            }
        }
        w.write("}\n");
    }

    void write_interface(writer& w, TypeDef const& type)
    {
        w.write("public interface @\n{\n", type.TypeName());
        {
            writer::indent_guard g{ w };
            write_members(w, type);
        }
        w.write("}\n");
    }

    void write_struct(writer& w, TypeDef const& type)
    {
        w.write("public struct @\n{\n}\n", type.TypeName());
    }

    void write_type(writer& w, TypeDef const& type)
    {
        switch (get_category(type))
        {
        case category::class_type:
            write_class(w, type);
            break;
        case category::delegate_type:
            write_delegate(w, type);
            break;
        case category::enum_type:
            write_enum(w, type);
            break;
        case category::interface_type:
            write_interface(w, type);
            break;
        case category::struct_type:
            write_struct(w, type);
            break;
        }
    }

    void write_type(TypeDef const& type)
    {
        if (is_exclusive_to(type))
        {
            return;
        }

        writer w;
        w.write("namespace %\n{\n", type.TypeNamespace());
        {
            writer::indent_guard g{ w };
            write_type(w, type);
        }
        w.write("}\n");

        auto filename = w.write_temp("%.@.cs", type.TypeNamespace(), type.TypeName());
        w.flush_to_file(settings.output_folder / filename);

    }
}