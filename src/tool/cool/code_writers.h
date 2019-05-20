#pragma once

namespace coolrt
{
    using namespace xlang::meta::reader;

	bool is_unsafe(method_signature const& signature)
	{
		if (signature.return_signature())
		{
			return true;
		}

		// I'm sure there are other scenarios where methods can be unsafe, 
		// but I'll figure them out later

		return false;
	}

	void write_throw_not_impl(writer& w)
	{
		w.write("throw new System.NotImplementedException();\n");
	}

	void write_constant(writer& w, Constant const& value)
	{
		switch (value.Type())
		{
		case ConstantType::Int32:
			w.write_printf("%d", value.ValueInt32());
			break;
		case ConstantType::UInt32:
			w.write_printf("%#0x", value.ValueUInt32());
			break;
		}
	}

	void write_fundamental_type(writer& w, fundamental_type type)
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
	}

    void write_projection_type(writer& w, type_semantics const& semantics)
    {
        call(semantics,
            [&](object_type) { w.write("object"); },
            [&](guid_type) { w.write("System.Guid"); },
            [&](type_definition const& type) { w.write("%.@", type.TypeNamespace(), type.TypeName()); },
            [&](generic_type_index const& var) { w.write(w.generic_param_stack.back()[var.index]); },
            [&](generic_type_instance const& type)
            {
				write_projection_type(w, type.generic_type);
                w.write("<");
                separator s{ w };
                for (auto&& type_arg : type.generic_args)
                {
                    s();
					write_projection_type(w, type_arg);
                }
                w.write(">");
            },
			[&](fundamental_type const& type) { write_fundamental_type(w, type); });
    }

    void write_type_name(writer& w, TypeDef const& type)
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

	void write_type_inheritance(writer& w, TypeDef const& type)
	{
		separator _s{ w };
		bool colon_written = false;
		auto s = [&]()
		{
			if (!colon_written)
			{
				w.write(" : ");
				colon_written = true;
			}
			_s();
		};

		if (get_category(type) == category::class_type && !is_static(type))
		{
			auto base_semantics = get_type_semantics(type.Extends());

			if (!std::holds_alternative<object_type>(base_semantics))
			{
				s();
				w.write(bind<write_projection_type>(base_semantics));
			}

			s();
			w.write("System.IDisposable");
		}

		for (auto&& iface : type.InterfaceImpl())
		{
			call(get_type_semantics(iface.Interface()),
				[&](type_definition const& type)
				{
					if (!is_exclusive_to(type))
					{
						s();
						w.write("%", bind<write_projection_type>(type));
					}
				},
				[&](generic_type_instance const& type)
				{
					if (!is_exclusive_to(type.generic_type))
					{
						s();
						w.write("%", bind<write_projection_type>(type));
					}
				},
					[](auto) { throw_invalid("invalid interface impl type"); });
		}
	}

	void write_parameter_name(writer& w, method_signature::param_t const& param)
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

        if (std::find(keywords.begin(), keywords.end(), param.first.Name()) != keywords.end())
        {
            w.write("@");
        }

        w.write(param.first.Name());
    }

	void write_iid_field(writer& w, TypeDef const& type)
	{
		auto attribute = get_attribute(type, "Windows.Foundation.Metadata", "GuidAttribute");
		if (!attribute)
		{
			throw_invalid("'Windows.Foundation.Metadata.GuidAttribute' attribute for type '", type.TypeNamespace(), ".", type.TypeName(), "' not found");
		}

		auto args = attribute.Value().FixedArgs();

		using std::get;

		auto get_arg = [&](decltype(args)::size_type index) { return get<ElemSig>(args[index].value).value; };

		w.write_printf("public static readonly Guid IID = new Guid(0x%08Xu,0x%04X,0x%04X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X);\n\n",
			get<uint32_t>(get_arg(0)),
			get<uint16_t>(get_arg(1)),
			get<uint16_t>(get_arg(2)),
			get<uint8_t>(get_arg(3)),
			get<uint8_t>(get_arg(4)),
			get<uint8_t>(get_arg(5)),
			get<uint8_t>(get_arg(6)),
			get<uint8_t>(get_arg(7)),
			get<uint8_t>(get_arg(8)),
			get<uint8_t>(get_arg(9)),
			get<uint8_t>(get_arg(10)));
	}














	void write_projection_parameter_type(writer& w, method_signature::param_t const& param)
	{
		auto semantics = get_type_semantics(param.second->Type());

		switch (get_param_category(param))
		{
		case param_category::in:
			w.write("%", bind<write_projection_type>(semantics));
			break;
		case param_category::out:
			w.write("out %", bind<write_projection_type>(semantics));
			break;
		case param_category::pass_array:
			w.write("/*pass_array*/ %[]", bind<write_projection_type>(semantics));
			break;
		case param_category::fill_array:
			w.write("/*fill_array*/ %[]", bind<write_projection_type>(semantics));
			break;
		case param_category::receive_array:
			w.write("/*receive_array*/ %[]", bind<write_projection_type>(semantics));
			break;
		}
	}

	void write_method_return(writer& w, method_signature const& signature)
	{
		if (signature.return_signature())
		{
			auto semantics = get_type_semantics(signature.return_signature().Type());
			write_projection_type(w, semantics);
		}
		else
		{
			w.write("void");
		}
	}

	void write_method_parameters(writer& w, method_signature const& signature)
	{
		separator s{ w };
		for (auto&& param : signature.params())
		{
			s();
			w.write("% %",
				bind<write_projection_parameter_type>(param),
				bind<write_parameter_name>(param));
		}
	}

	void write_class_factory(writer& w, TypeDef const& type, activation_factory const& factory)
	{
		if (factory.type)
		{
			for (auto&& method : factory.type.MethodList())
			{
				method_signature signature{ method };
				w.write("public %(%)\n{\n", type.TypeName(), bind<write_method_parameters>(signature));
				{
					writer::indent_guard gg{ w };
					write_throw_not_impl(w);
				}
				w.write("}\n");
			}
		}
		else
		{
			w.write("public %()\n{\n", type.TypeName());
			{
				writer::indent_guard gg{ w };
				write_throw_not_impl(w);
			}
			w.write("}\n");
			//w.write(strings::default_activation, type.TypeName());
		}
	}

	void write_class_method(writer& w, MethodDef const& method, bool is_static)
	{
		if (method.Flags().SpecialName())
		{
			return	;
		}

		method_signature signature{ method };
		w.write("public %% %(%)\n{\n",
			is_static ? "static " : "",
			bind<write_method_return>(signature),
			method.Name(),
			bind<write_method_parameters>(signature));
		{
			writer::indent_guard g{ w };
			write_throw_not_impl(w);
		}
		w.write("}\n");
	}

	void write_class_property(writer& w, Property const& prop, bool is_static)
	{
		// TODO: WinRT requires property getters and allows setters to be added via a seperate interface

		auto [getter, setter] = get_property_methods(prop);

		w.write("public %% %\n{\n", 
			is_static ? "static /*prop*/ " : "",
			bind<write_projection_type>(get_type_semantics(prop.Type().Type())),
			prop.Name());

		{
			writer::indent_guard g{ w };

			if (getter)
			{
				w.write("get\n{\n");
				{
					writer::indent_guard gg{ w };
					write_throw_not_impl(w);
				}
				w.write("}\n");
			}

			if (setter)
			{
				w.write("set\n{\n");
				{
					writer::indent_guard gg{ w };
					write_throw_not_impl(w);
				}
				w.write("}\n");
			}
		}
		w.write("}\n");
	}

	void write_class_event(writer& w, Event const& event, bool is_static)
	{
		w.write("public %event % %\n{\n",
			is_static ? "static " : "",
			bind<write_projection_type>(get_type_semantics(event.EventType())),
			event.Name());
		{
			writer::indent_guard g{ w };
			w.write("add\n{\n");
			{
				writer::indent_guard gg{ w };
				write_throw_not_impl(w);
			}
			w.write("}\n");

			w.write("remove\n{\n");
			{
				writer::indent_guard gg{ w };
				write_throw_not_impl(w);
			}
			w.write("}\n");
		}
		w.write("}\n");

	}

	void write_class_factory(writer& w, TypeDef const& /*type*/, static_factory const& factory)
	{
		XLANG_ASSERT(factory.type);

		w.write_each<write_class_method>(factory.type.MethodList(), true);
		w.write_each<write_class_property>(factory.type.PropertyList(), true);
		w.write_each<write_class_event>(factory.type.EventList(), true);
	}

    void write_class(writer& w, TypeDef const& type)
    {
        w.write("public %class %%\n{\n", 
			is_static(type) ? "static " : "", 
			bind<write_type_name>(type), 
			bind<write_type_inheritance>(type));
		{
			writer::indent_guard g{ w };

			if (!is_static(type))
			{
				w.write("public void Dispose()\n{\n    throw new System.NotImplementedException();\n}\n");
			}

			//w.write("static System.Lazy<System.IntPtr> _factory = new System.Lazy<System.IntPtr>(() => __Interop__.Windows.Foundation.Platform.GetActivationFactory(\"%.%\"));\n",
			//	type.TypeNamespace(), type.TypeName());

			//if (!is_static(type))
			//{
			//	w.write("private System.IntPtr _instance;\n");
			//	w.write(strings::dispose_pattern, type.TypeName());
			//}

			for (auto&& factory : get_factories(type))
			{
				call(factory, [&](auto const& info) { write_class_factory(w, type, info); });
			}

			for (auto&& ii : type.InterfaceImpl())
			{
				auto semantics = get_type_semantics(ii.Interface());
				auto interface_type = get_typedef(semantics);

				if (is_istringable(interface_type))
				{
					auto format = R"(public override string ToString()
{
    throw new System.NotImplementedException();
})";
					w.write(format);
					continue;
				}

				auto guard{ w.push_generic_params(semantics, [&](auto const& arg) { return w.write_temp("%", bind<write_projection_type>(arg)); }) };
				auto default_interface = has_attribute(ii, "Windows.Foundation.Metadata", "DefaultAttribute");

				w.write_each<write_class_method>(interface_type.MethodList(), false);
				w.write_each<write_class_property>(interface_type.PropertyList(), false);
				w.write_each<write_class_event>(interface_type.EventList(), false);
			}
        }
        w.write("}\n");
    }

    void write_interface(writer& w, TypeDef const& type)
    {
        w.write("public interface %%\n{\n", 
			bind<write_type_name>(type), 
			bind<write_type_inheritance>(type));
        {
            writer::indent_guard g{ w };
            for (auto&& method : type.MethodList())
            {
                if (is_special(method))
                {
                    continue;
                }

				method_signature signature{ method };
				w.write("% %(%);\n", bind<write_method_return>(signature), method.Name(), bind<write_method_parameters>(signature));
            }

            for (auto&& prop : type.PropertyList())
            {
                auto [getter, setter] = get_property_methods(prop);

                auto semantics = get_type_semantics(prop.Type().Type());
                w.write("% % { %%}\n", bind<write_projection_type>(semantics), prop.Name(),
                    getter ? "get; " : "",
                    setter ? "set; " : "");
            }

            for (auto&& evt : type.EventList())
            {
                auto semantics = get_type_semantics(evt.EventType());
                w.write("event % %;\n", bind<write_projection_type>(semantics), evt.Name());
            }
        }
        w.write("}\n");
    }

	void write_delegate(writer& w, TypeDef const& type)
	{
		method_signature signature{ get_delegate_invoke(type) };
		w.write("public delegate % %(%);\n", 
			bind<write_method_return>(signature), 
			bind<write_type_name>(type), 
			bind<write_method_parameters>(signature));
	}

	void write_enum(writer& w, TypeDef const& type)
	{
		if (is_flags_enum(type))
		{
			w.write("[System.FlagsAttribute]");
		}

		w.write("public enum % : %\n{\n", bind<write_type_name>(type), is_flags_enum(type) ? "int" : "uint");
		{
			writer::indent_guard g{ w };

			for (auto&& field : type.FieldList())
			{
				if (auto constant = field.Constant())
				{
					w.write("% = %,\n", field.Name(), bind<write_constant>(constant));
				}
			}
		}
		w.write("}\n");
	}

	void write_struct(writer& w, TypeDef const& type)
    {
        w.write("public struct %\n{\n", bind<write_type_name>(type));
        {
            writer::indent_guard g{ w };

            for (auto&& field : type.FieldList())
            {
                auto semantics = get_type_semantics(field.Signature().Type());
                w.write("public % %;\n", bind<write_projection_type>(semantics), field.Name());
            }
        }
        w.write("}\n");
    }

    void write_projection_type(writer& w, TypeDef const& type)
    {
		auto write = [&](auto func)
		{
			w.write("namespace %\n{\n", type.TypeNamespace());
			{
				writer::indent_guard g{ w };
				func(w, type);
			}
			w.write("}\n");
		};
		
		switch (get_category(type))
        {
        case category::class_type:
			write(write_class);
            break;
        case category::delegate_type:
			write(write_delegate);
            break;
        case category::enum_type:
			write(write_enum);
            break;
        case category::interface_type:
			if (!is_exclusive_to(type))
			{
				write(write_interface);
			}
            break;
        case category::struct_type:
			write(write_struct);
            break;
        }
    }

	void write_interop_type(writer& w, type_semantics const& semantics)
	{
		call(semantics,
			[&](object_type) { w.write("IntPtr"); },
			[&](guid_type) { w.write("System.Guid"); },
			[&](type_definition const&) { w.write("IntPtr"); },
			[&](generic_type_index const& var) { w.write(w.generic_param_stack.back()[var.index]); },
			[&](generic_type_instance const&)
			{
				w.write("IntPtr");
			},
			[&](fundamental_type const& type)
			{
				if (type == fundamental_type::String)
				{
					w.write("IntPtr");
				}
				else
				{
					write_fundamental_type(w, type);
				}
			});
	}

	void write_interop_parameter_type(writer& w, method_signature::param_t const& param)
	{
		auto semantics = get_type_semantics(param.second->Type());

		switch (get_param_category(param))
		{
		case param_category::in:
			w.write("%", bind<write_interop_type>(semantics));
			break;
		case param_category::out:
			w.write("/*out*/ %", bind<write_interop_type>(semantics));
			break;
		case param_category::pass_array:
			w.write("/*pass_array*/ %[]", bind<write_interop_type>(semantics));
			break;
		case param_category::fill_array:
			w.write("/*fill_array*/ %[]", bind<write_interop_type>(semantics));
			break;
		case param_category::receive_array:
			w.write("/*receive_array*/ %[]", bind<write_interop_type>(semantics));
			break;
		}
	}

	void write_interop_return(writer& w, method_signature const& signature)
	{
		if (signature.return_signature())
		{
			write_interop_type(w, get_type_semantics(signature.return_signature().Type()));
		}
		else
		{
			w.write("void");
		}
	}

	void write_interop_parameters(writer& w, method_signature const& signature)
	{
		w.write("IntPtr @this");

		for (auto&& param : signature.params())
		{
			w.write(", % %", bind<write_interop_parameter_type>(param), bind<write_parameter_name>(param));
		}
	}

	void write_abi_parameters(writer& w, method_signature const& signature)
	{
		write_interop_parameters(w, signature);

		if (signature.return_signature())
		{
			auto semantics = get_type_semantics(signature.return_signature().Type());
			w.write(", %* %", bind<write_interop_type>(semantics), signature.return_param_name());
		}
	}

	void write_interop_method_name(writer& w, MethodDef const& method, std::string_view prefix, int offset)
	{
		w.write("%%%", prefix, method.Name(), offset);
	}

	void write_interop_interface(writer& w, TypeDef const& type)
	{
		XLANG_ASSERT(get_category(type) == category::interface_type);

		if (is_ptype(type))
		{
			w.write("// parameterized interfaces TBD\n");
			return;
		}

		w.write("internal static class @\n{\n", type.TypeName());
		{
			writer::indent_guard g{ w };
			write_iid_field(w, type);

			int offset = 5; // start offset @ 5 so we can increment at top of loop
			for (auto&& method : type.MethodList())
			{
				offset++;

				method_signature signature{ method };
				w.write("private %delegate int %(%);\n",
					is_unsafe(signature) ? "unsafe " : "",
					bind<write_interop_method_name>(method, "abi", offset),
					bind<write_abi_parameters>(signature));

				w.write("private static unsafe int %(%)\n{\n", 
					bind<write_interop_method_name>(method, "invoke", offset),
					bind<write_abi_parameters>(signature));
				{
					writer::indent_guard gg{ w };
					w.write("void* __slot = (*(void***)^@this.ToPointer())[%];\n", offset);
					w.write("var __delegate = Marshal.GetDelegateForFunctionPointer<%>(new IntPtr(__slot));\n", 
						bind<write_interop_method_name>(method, "abi", offset));

					w.write("return __delegate(@this");
					for (auto&& param : signature.params())
					{
						w.write(", %", bind<write_parameter_name>(param));
					}
					if (signature.return_signature())
					{
						w.write(", %", signature.return_param_name());
					}
					w.write(");\n");
				}
				w.write("}\n");

				w.write("internal static %% %(%)\n{\n", 
					is_unsafe(signature) ? "unsafe " : "",
					bind<write_interop_return>(signature),
					bind<write_interop_method_name>(method, "", offset),
					bind<write_interop_parameters>(signature));
				{
					writer::indent_guard gg{ w };

					auto write_invoke = [&]() 
					{
						w.write("Marshal.ThrowExceptionForHR(%(^@this", 
							bind<write_interop_method_name>(method, "invoke", offset));
						for (auto&& param : signature.params())
						{
							w.write(", %", bind<write_parameter_name>(param));
						}
						if (signature.return_signature())
						{
							w.write(", &%", signature.return_param_name());
						}
						w.write("));\n");
					};

					if (signature.return_signature())
					{
						auto semantics = get_type_semantics(signature.return_signature().Type());

						w.write("% % = default;\n",
							bind<write_interop_type>(semantics),
							signature.return_param_name());
						write_invoke();
						w.write("return %;\n", signature.return_param_name());
					}
					else
					{
						write_invoke();
					}
				}
				w.write("}\n");
			}
		}
		w.write("}\n");
	}

	void write_interop_delegate(writer& /*w*/, TypeDef const& /*type*/)
	{

	}
	
	void write_interop_type(writer& w, TypeDef const& type)
    {
		auto write = [&](auto func)
		{
			w.write("namespace __Interop__.%\n{\n", type.TypeNamespace());
			{
				writer::indent_guard g{ w };
				w.write("using System;\nusing System.Runtime.InteropServices;\n\n");
				func(w, type);
			}
			w.write("}\n");
		};

		switch (get_category(type))
		{
		case category::delegate_type:
			write(write_interop_delegate);
			break;
		case category::interface_type:
			write(write_interop_interface);
			break;
		}
    }

    void write_type(TypeDef const& type)
    {
        if (is_api_contract_type(type)) { return; }
        if (is_attribute_type(type)) { return; }

        writer w;
        auto guard{ w.push_generic_params(type.GenericParam()) };

		write_projection_type(w, type);
		write_interop_type(w, type);

        auto filename = w.write_temp("%.@.cs", type.TypeNamespace(), type.TypeName());
        w.flush_to_file(settings.output_folder / filename);
    }
}