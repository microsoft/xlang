#pragma once

namespace coolrt
{
    using namespace xlang::meta::reader;

    void write_projected_type(writer& w, type_semantics const& semantics)
    {
        call(semantics,
            [&](object_type) { w.write("object"); },
            [&](guid_type) { w.write("System.Guid"); },
            [&](type_definition const& type) { w.write("%.@", type.TypeNamespace(), type.TypeName()); },
            [&](generic_type_index const& var) { w.write(w.generic_param_stack.back()[var.index]); },
            [&](generic_type_instance const& type)
            {
                write_projected_type(w, type.generic_type);
                w.write("<");
                separator s{ w };
                for (auto&& type_arg : type.generic_args)
                {
                    s();
                    write_projected_type(w, type_arg);
                }
                w.write(">");
            },
            [&](fundamental_type const& type)
            {
                switch (type)
                {
                case fundamental_type::Boolean:
                    w.write("bool");
                    break;
                case fundamental_type::Char:
                    w.write("char");
                    break;
                case fundamental_type::Int8:
                    w.write("sbyte");
                    break;
                case fundamental_type::UInt8:
                    w.write("byte");
                    break;
                case fundamental_type::Int16:
                    w.write("short");
                    break;
                case fundamental_type::UInt16:
                    w.write("ushort");
                    break;
                case fundamental_type::Int32:
                    w.write("int");
                    break;
                case fundamental_type::UInt32:
                    w.write("uint");
                    break;
                case fundamental_type::Int64:
                    w.write("long");
                    break;
                case fundamental_type::UInt64:
                    w.write("ulong");
                    break;
                case fundamental_type::Float:
                    w.write("float");
                    break;
                case fundamental_type::Double:
                    w.write("double");
                    break;
                case fundamental_type::String:
                    w.write("string");
                    break;
                default:
                    throw_invalid("invalid fundamental type");
                }
            });
    }

    void write_interop_type(writer& w, type_semantics const& semantics)
    {
        w.write("/*INTEROP*/ ");
        call(semantics,
            [&](object_type) { w.write("object"); },
            [&](guid_type) { w.write("System.Guid"); },
            [&](type_definition const& type) { w.write("%.@", type.TypeNamespace(), type.TypeName()); },
            [&](generic_type_index const& var) { w.write(w.generic_param_stack.back()[var.index]); },
            [&](generic_type_instance const& type)
            {
                write_projected_type(w, type.generic_type);
                w.write("<");
                separator s{ w };
                for (auto&& type_arg : type.generic_args)
                {
                    s();
                    write_projected_type(w, type_arg);
                }
                w.write(">");
            },
            [&](fundamental_type const& type)
            {
                switch (type)
                {
                case fundamental_type::Boolean:
                    w.write("bool");
                    break;
                case fundamental_type::Char:
                    w.write("char");
                    break;
                case fundamental_type::Int8:
                    w.write("sbyte");
                    break;
                case fundamental_type::UInt8:
                    w.write("byte");
                    break;
                case fundamental_type::Int16:
                    w.write("short");
                    break;
                case fundamental_type::UInt16:
                    w.write("ushort");
                    break;
                case fundamental_type::Int32:
                    w.write("int");
                    break;
                case fundamental_type::UInt32:
                    w.write("uint");
                    break;
                case fundamental_type::Int64:
                    w.write("long");
                    break;
                case fundamental_type::UInt64:
                    w.write("ulong");
                    break;
                case fundamental_type::Float:
                    w.write("float");
                    break;
                case fundamental_type::Double:
                    w.write("double");
                    break;
                case fundamental_type::String:
                    w.write("string");
                    break;
                default:
                    throw_invalid("invalid fundamental type");
                }
            });
    }

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
        auto semantics = get_type_semantics(param.second->Type());

        switch (get_param_category(param))
        {
        case param_category::in:
            w.write("%", bind<write_projected_type>(semantics));
            break;
        case param_category::out:
            w.write("out %", bind<write_projected_type>(semantics));
            break;
        case param_category::pass_array:
            w.write("/*pass_array*/ %[]", bind<write_projected_type>(semantics));
            break;
        case param_category::fill_array:
            w.write("/*fill_array*/ %[]", bind<write_projected_type>(semantics));
            break;
        case param_category::receive_array:
            w.write("/*receive_array*/ %[]", bind<write_projected_type>(semantics));
            break;
        }
    }

    void write_parameter_name(writer& w, std::string_view name)
    {
        static const std::set<std::string_view> keywords = {
            "abstract",  "as",       "base",     "bool",       "break",     "byte",
            "case",      "catch",    "char",     "checked",    "class",     "const",
            "continue",  "decimal",  "default",  "delegate",   "do",        "double",
            "else",      "enum",     "event",    "explicit",   "extern",    "false",
            "finally",   "fixed",    "float",    "for",        "foreach",   "goto",
            "if",        "implicit", "in",       "int",        "interface", "internal",
            "is",        "lock",     "long",     "namespace",  "new",       "null",
            "object",    "operator", "out",      "override",   "params",    "private",
            "protected", "public",   "readonly", "ref",        "return",    "sbyte",
            "sealed",    "short",    "sizeof",   "stackalloc", "static",    "string",
            "struct",    "switch",   "this",     "throw",      "true",      "try",
            "typeof",    "uint",     "ulong",    "unchecked",  "unsafe",    "ushort",
            "using",     "virtual",  "void",     "volatile",   "while" };

        if (std::find(keywords.begin(), keywords.end(), name) != keywords.end())
        {
            w.write("@");
        }

        w.write(name);
    }

    void write_method_parameters(writer& w, method_signature const& signature)
    {
        separator s{ w };
        for (auto&& param : signature.params())
        {
            s();
            w.write("% %", bind<write_parameter_type>(param), bind<write_parameter_name>(param.first.Name()));
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
            if (method.Flags().Static())
            {
                w.write("static ");
            }

            auto retsig = signature.return_signature();
            if (retsig)
            {
                auto semantics = get_type_semantics(retsig.Type());
                write_projected_type(w, semantics);
            }
            else
            {
                w.write("void");
            }
            w.write(" %", method.Name());
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
            w.write(bind<write_projected_type>(get_type_semantics(type.Extends())));
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
                        w.write("%", bind<write_projected_type>(type));
                    }
                },
                [&](generic_type_instance const& type)
                {
                    if (!is_exclusive_to(type.generic_type))
                    {
                        write_colon();
                        s();
                        w.write("%", bind<write_projected_type>(type));
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
                auto semantics = get_type_semantics(prop.Type().Type());
                w.write("public % %\n{\n", bind<write_projected_type>(semantics), prop.Name());
                {
                    writer::indent_guard gg{ w };
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
                auto semantics = get_type_semantics(evt.EventType());
                w.write("public event % %;\n", bind<write_projected_type>(semantics), evt.Name());
            }
        }
        w.write("}\n");
    }

    void write_delegate(writer& w, TypeDef const& type)
    {
        method_signature signature{ get_delegate_invoke(type) };
        w.write("public delegate void %(%);\n", bind<write_ptype_name>(type), bind<write_method_parameters>(signature));
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
                auto [getter, setter] = get_property_methods(prop);

                auto semantics = get_type_semantics(prop.Type().Type());
                w.write("% % { % % }\n", bind<write_projected_type>(semantics), prop.Name(),
                    getter ? "get;" : "",
                    setter ? "set;" : "");
            }

            for (auto&& evt : type.EventList())
            {
                auto semantics = get_type_semantics(evt.EventType());
                w.write("event % %;\n", bind<write_projected_type>(semantics), evt.Name());
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
                auto semantics = get_type_semantics(field.Signature().Type());
                w.write("public % %;\n", bind<write_projected_type>(semantics), field.Name());
            }
        }
        w.write("}\n");
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

    void write_interop_parameters(writer& w, method_signature const& signature)
    {
        for (auto&& param : signature.params())
        {
            // bind<write_parameter_type>(param)
            w.write(", % %", "IntPtr", bind<write_parameter_name>(param.first.Name()));
        }

        if (signature.return_signature())
        {
            w.write(", [Out] %* %", "int", signature.return_param_name("@return"));
        }
    }

    void write_interop_argument_names(writer& w, method_signature const& signature)
    {
        for (auto&& param : signature.params())
        {
            // bind<write_parameter_type>(param)
            w.write(", %", bind<write_parameter_name>(param.first.Name()));
        }

        if (signature.return_signature())
        {
            w.write(", %", signature.return_param_name("@return"));
        }
    }

    void write_interop_type(writer& w, TypeDef const& type)
    {
        XLANG_ASSERT(get_category(type) == category::interface_type);

        w.write("internal unsafe static class @\n{\n", type.TypeName());
        {
            writer::indent_guard g{ w };

            int offset = 6;
            for (auto&& method : type.MethodList())
            {
                method_signature signature{ method };
                w.write("delegate int delegate%(void* ^@this%);\n", method.Name(), bind<write_interop_parameters>(signature));
                w.write("public static int invoke%(void* ^@this%)\n{\n", method.Name(), bind<write_interop_parameters>(signature));
                {
                    writer::indent_guard gg{ w };
                    w.write("var __delegate = __Interop__.Helper.GetDelegate<delegate%>(^@this, %);\n", method.Name(), offset++);
                    w.write("return __delegate(^@this%);\n", bind<write_interop_argument_names>(signature));
                }
                w.write("}\n");
            }
        }
        w.write("}\n");
    }

    void write_type(TypeDef const& type)
    {
        if (is_api_contract_type(type)) { return; }
        if (is_attribute_type(type)) { return; }

        writer w;
        auto guard{ w.push_generic_params(type.GenericParam()) };

        if (!is_exclusive_to(type))
        {
            w.write("namespace %\n{\n", type.TypeNamespace());
            {
                writer::indent_guard g{ w };
                write_type(w, type);
            }
            w.write("}\n");
        }

        if (get_category(type) == category::interface_type)
        {
            w.write("namespace __Interop__.%\n{\n", type.TypeNamespace());
            w.write("#pragma warning disable 0649\n");
            {
                writer::indent_guard g{ w };
                w.write("using System;\nusing System.Runtime.InteropServices;\n\n");
                write_interop_type(w, type);
            }
            w.write("#pragma warning restore 0649\n");
            w.write("}\n");
        }

        auto filename = w.write_temp("%.@.cs", type.TypeNamespace(), type.TypeName());
        w.flush_to_file(settings.output_folder / filename);
    }
}