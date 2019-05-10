#pragma once

namespace coolrt
{
    using namespace xlang::meta::reader;

    void write_ptype_name(writer& w, TypeDef const& type)
    {
        w.write("@", type.TypeName());
        if (distance(type.GenericParam()) > 0)
        {
            w.write("<");
            separator s{ w };
            for (auto&& gp : type.GenericParam())
            {
                s();
                w.write(gp.Name());
            }
            w.write(">");
        }
    }

    void write_throw_not_impl(writer& w)
    {
        writer::indent_guard g{ w };
        w.write("throw new System.NotImplementedException();\n");
    }

    void write_parameter_type(writer& w, method_signature::param_t const& param)
    {
        switch (get_param_category(param))
        {
        case param_category::in:
            w.write(param.second->Type());
            break;
        case param_category::out:
            w.write("out %", param.second->Type());
            break;
        case param_category::pass_array:
            w.write("/*pass_array*/ %[]", param.second->Type());
            break;
        case param_category::fill_array:
            w.write("/*fill_array*/ %[]", param.second->Type());
            break;
        case param_category::receive_array:
            w.write("/*receive_array*/ %[]", param.second->Type());
            break;
        }
    }

    void write_method_parameters(writer& w, method_signature const& signature)
    {
        separator s{ w };
        for (auto&& param : signature.params())
        {
            s();
            w.write("% %", bind<write_parameter_type>(param), param.first.Name());
        }
    }

    void write_method_signature(writer& w, MethodDef const& method, method_signature const& signature)
    {
        if (is_constructor(method))
        {
            w.write(method.Parent().TypeName());
        }
        else
        {
            w.write("% %", signature.return_signature(), method.Name());
        }

        w.write("(%)", bind<write_method_parameters>(signature));
    }

    void write_type_inheritance(writer& w, TypeDef const& type)
    {
        bool colon_written = false;
        auto write_colon = [&]()
        {
            if (!colon_written)
            {
                w.write(" : ");
                colon_written = true;
            }
        };

        separator s{ w };

        if (get_category(type) == category::class_type)
        {
            write_colon();
            s();
            w.write(type.Extends());
        }

        for (auto&& iface : type.InterfaceImpl())
        {
            call(get_type_semantics(iface.Interface()),
                [&](type_definition const& type) 
                { 
                    if (!is_exclusive_to(type))
                    {
                        write_colon();
                        s();
                        w.write(type);
                    }
                },
                [&](generic_type_instance const& type) 
                { 
                    if (!is_exclusive_to(type.generic_type))
                    {
                        write_colon();
                        s();
                        w.write(type);
                    }
                },
                [](auto) { throw_invalid("invalid interface impl type"); });
        }
    }

    void write_class(writer& w, TypeDef const& type)
    {
        w.write("public class @%\n{\n", type.TypeName(), bind<write_type_inheritance>(type));
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
                w.write("public %\n{\n", bind<write_method_signature>(method, signature));
                write_throw_not_impl(w);
                w.write("}\n");
            }

            for (auto&& prop : type.PropertyList())
            {
                auto [getter, setter] = get_property_methods(prop);
                w.write("public % %\n{\n", prop.Type(), prop.Name());
                {
                    writer::indent_guard g{ w };
                    w.write("get { throw new System.NotImplementedException(); }\n");
                    if (setter)
                    {
                        w.write("set { throw new System.NotImplementedException(); }\n");
                    }
                }
                w.write("}\n");
            }

            for (auto&& evt : type.EventList())
            {
                w.write("public event % %;\n", evt.EventType(), evt.Name());
            }
        }
        w.write("}\n");
    }

    void write_delegate(writer& w, TypeDef const& type)
    {
        method_signature signature{ get_delegate_invoke(type) };

        w.write("public delegate void %();\n", bind<write_ptype_name>(type));
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
        w.write("public interface %%\n{\n", bind<write_ptype_name>(type), bind<write_type_inheritance>(type));
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
                w.write("% % { get; set; }\n", prop.Type(), prop.Name());
            }

            for (auto&& evt : type.EventList())
            {
                w.write("event % %;\n", evt.EventType(), evt.Name());
            }
        }
        w.write("}\n");
    }

    void write_struct(writer& w, TypeDef const& type)
    {
        w.write("public struct @\n{\n", type.TypeName());
        {
            writer::indent_guard g{ w };

            for (auto&& field : type.FieldList())
            {
                w.write("public % %;\n", field.Signature().Type(), field.Name());
            }
        }
        w.write("}\n");
    }

    void write_type(writer& w, TypeDef const& type)
    {
        auto guard{ w.push_generic_params(type.GenericParam()) };

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
        if (is_exclusive_to(type)) { return; }
        if (is_api_contract_type(type)) { return; }
        if (is_attribute_type(type)) { return; }

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