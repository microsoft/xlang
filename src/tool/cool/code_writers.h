#pragma once

namespace coolrt
{
    using namespace xlang::meta::reader;

    void write_throw_not_impl(writer& w)
    {
        writer::indent_guard g{ w };
        w.write("throw new System.NotImplementedException();\n");
    }

    void write_method_parameters(writer& w, method_signature const& signature)
    {
        separator s{ w };
        for (auto&& [param, param_sig] : signature.params())
        {
            s();
            w.write("% %", param_sig, param.Name());
        }
    }

    void write_method_signature(writer& w, MethodDef const& method, method_signature const& signature)
    {
        if (!is_constructor(method))
        {
            w.write("% ", signature.return_signature());
        }

        w.write("%(%)", method.Name(), bind<write_method_parameters>(signature));
    }

    void write_class(writer& w, TypeDef const& type)
    {
        w.write("public class @\n{\n", type.TypeName());
        {
            writer::indent_guard g{ w };
            for (auto&& method : type.MethodList())
            {
                if (!is_constructor(method))
                {
                    continue;
                }

                method_signature signature{ method };
                w.write("public %\n{\n", bind<write_method_signature>(method, signature));
                write_throw_not_impl(w);
                w.write("}\n");
            }

            for (auto&& method : type.MethodList())
            {
                if (is_special(method))
                {
                    continue;
                }

                method_signature signature{ method };
                w.write("public % %(%)\n{\n", 
                    signature.return_signature(),
                    method.Name(), 
                    bind<write_method_parameters>(signature));
                write_throw_not_impl(w);
                w.write("}\n");
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
            for (auto&& method : type.MethodList())
            {
                if (is_special(method))
                {
                    continue;
                }

                method_signature signature{ method };
                w.write("%;\n", bind<write_method_signature>(method, signature));
            }

            for (auto&& prop : type.PropertyList())
            {
                w.write("// prop %\n", prop.Name());
            }

            for (auto&& evt : type.EventList())
            {
                w.write("// evt %\n", evt.Name());
            }
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