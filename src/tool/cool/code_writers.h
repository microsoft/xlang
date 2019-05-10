#pragma once

namespace coolrt
{
    using namespace xlang::meta::reader;

    bool is_exclusive_to(TypeDef const& type)
    {
        return get_category(type) == category::interface_type && get_attribute(type, "Windows.Foundation.Metadata", "ExclusiveToAttribute");
    }

    bool is_flags_enum(TypeDef const& type)
    {
        return get_category(type) == category::enum_type && get_attribute(type, "System", "FlagsAttribute");
    }

    void write_class(writer& w, TypeDef const& type)
    {
        w.write("public class @\n{\n}\n", type.TypeName());
    }

    void write_delegate(writer& w, TypeDef const& type)
    {
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
        if (!is_exclusive_to(type))
        {
            w.write("public interface @\n{\n}\n", type.TypeName());
        }

        //w.write("\nnamespace _Interop\n{\n");
        //{
        //    writer::indent_guard g{ w };
        //    w.write("private interface @\n{\n}\n", type.TypeName());
        //}

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