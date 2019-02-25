#include "pch.h"
#include "settings.h"
#include "writer.h"

namespace rgm
{
	using namespace xlang;
	using namespace xlang::text;
	using namespace xlang::meta::reader;

	struct method_signature
	{
		using param_t = std::pair<Param, ParamSig const*>;

		explicit method_signature(MethodDef const& method) :
			m_method(method.Signature())
		{
			auto params = method.ParamList();

			if (m_method.ReturnType() && params.first != params.second && params.first.Sequence() == 0)
			{
				m_return = params.first;
				++params.first;
			}

			for (uint32_t i{}; i != m_method.Params().size(); ++i)
			{
				m_params.emplace_back(params.first + i, m_method.Params().data() + i);
			}
		}

		std::vector<param_t>& params()
		{
			return m_params;
		}

		std::vector<param_t> const& params() const
		{
			return m_params;
		}

		auto const& return_signature() const
		{
			return m_method.ReturnType();
		}

		auto return_param_name() const
		{
			std::string_view name;

			if (m_return)
			{
				name = m_return.Name();
			}
			else
			{
				name = "winrt_impl_return_value";
			}

			return name;
		}

		bool has_params() const
		{
			return !m_params.empty();
		}

	private:

		MethodDefSig m_method;
		std::vector<param_t> m_params;
		Param m_return;
	};

	settings_type settings;

	struct usage_exception {};

	static constexpr cmd::option options[]
	{
		{ "input", 1, 1, "<path>", "WinMD file to dump as il" },
		{ "output", 0, 1, "<path>", "Location of generated il file" },
	};

	void process_args(int const argc, char** argv)
	{
		namespace fs = std::experimental::filesystem;
		cmd::reader args{ argc, argv, options };

		if (!args)
		{
			throw usage_exception{};
		}

		settings.input = args.value("input");
		XLANG_ASSERT(database::is_database(settings.input));
		auto p = fs::canonical(fs::path{"."} / fs::path{ settings.input }.filename().replace_extension(".il"));
		settings.output = args.value("output", p.string());
	}

	static void print_usage(writer& w)
	{
		static auto printColumns = [](writer& w, std::string_view const& col1, std::string_view const& col2)
		{
			w.write_printf("  %-20s%s\n", col1.data(), col2.data());
		};

		static auto printOption = [](writer& w, cmd::option const& opt)
		{
			if (opt.desc.empty())
			{
				return;
			}
			printColumns(w, w.write_temp("-% %", opt.name, opt.arg), opt.desc);
		};

		auto format = R"(
RGM/xlang v%
Copyright (c) Microsoft Corporation. All rights reserved.

  rgm.exe [options...]

Options:

%
)";
		w.write(format, "0.0.1", bind_each(printOption, options));
	}

	static auto get_files_to_cache()
	{
		std::vector<std::string> files;
		files.push_back(settings.input);
		return files;
	}

	static void write_if_true(writer& w, bool value, std::string_view text)
	{
		if (value)
		{
			w.write(text);
		}
	}

	void write_enum_constant(writer& w, Constant const& constant)
	{
		if (constant)
		{
			w.write(" = %", constant);
		}

	}

	static void write_enum(writer& w, TypeDef const& type)
	{
		//TODO: Flag enum support

		XLANG_ASSERT(type.Flags().Semantics() == TypeSemantics::Class);
		auto guard{ w.push_generic_params(type.GenericParam()) };

		w.write(".class % % % %%%.% extends [mscorlib]System.Enum\n{\n",
			type.Flags().Visibility(),
			type.Flags().Layout(),
			type.Flags().StringFormat(),
			bind<write_if_true>(type.Flags().WindowsRuntime(), "windowsruntime "),
			bind<write_if_true>(type.Flags().Sealed(), "sealed "),
			type.TypeNamespace(),
			type.TypeName());

		for (auto&& field : type.FieldList())
		{
			w.write("    .field % %%%%% %%",
				field.Flags().Access(),
				bind<write_if_true>(field.Flags().Static(), "static "),
				bind<write_if_true>(field.Flags().SpecialName(), "specialname "),
				bind<write_if_true>(field.Flags().RTSpecialName(), "rtspecialname "),
				bind<write_if_true>(field.Flags().Literal(), "literal "),
				field.Signature().Type(),
				field.Name(),
				bind<write_enum_constant>(field.Constant()));
			w.write("\n");
		}
		w.write("}\n\n");
	}

	static void write_struct(writer& w, TypeDef const& type)
	{
		XLANG_ASSERT(type.Flags().Semantics() == TypeSemantics::Class);
		auto guard{ w.push_generic_params(type.GenericParam()) };

		w.write(".class % % % %%%.% extends [mscorlib]System.ValueType\n{\n",
			type.Flags().Visibility(),
			type.Flags().Layout(),
			type.Flags().StringFormat(),
			bind<write_if_true>(type.Flags().WindowsRuntime(), "windowsruntime "),
			bind<write_if_true>(type.Flags().Sealed(), "sealed "),
			type.TypeNamespace(),
			type.TypeName());

		for (auto&& field : type.FieldList())
		{
			w.write("    .field % % %\n", 
				type.Flags().Visibility(),
				field.Signature().Type(),
				field.Name());
		}

		w.write("}\n\n");
	}

	static void write_generic_params(writer& w, std::pair<GenericParam, GenericParam> const& params)
	{
		if (params.first != params.second)
		{
			w.write("<%>", bind_list(",", params));
		}
	}


	inline bool operator && (CallingConvention lhs, CallingConvention rhs)
	{
		using T = std::underlying_type_t <CallingConvention>;
		return (static_cast<T>(lhs) & static_cast<T>(rhs)) != 0;
	}

	static void write_method(writer& w, MethodDef const& method)
	{
		method_signature signature{ method };

		w.write(R"(  .method % %% %%%%
    % % % ()",
			method.Flags().Access(),
			method.Flags().HideBySig() ? "hidebysig " : "",
			method.Flags().Layout(),
			method.Flags().SpecialName() ? "specialname " : "",
			method.Flags().RTSpecialName() ? "rtspecialname " : "",
			method.Flags().Abstract() ? "abstract " : "",
			method.Flags().Virtual() ? "virtual " : "",
			method.Flags().Static() ? "static" : "instance",
			signature.return_signature(),
			method.Name());

		if (!signature.params().empty())
		{
			w.write("\n      %", bind_list(",\n      ", signature.params()));
		}

		w.write(R"() % %
  {
  }
)",
			method.ImplFlags().CodeType(),
			method.ImplFlags().Managed());
	}

	static void write_semantic(writer& w, MethodSemantics const& semantics)
	{
		if (semantics.Semantic().Getter())
		{
			w.write("get");
		}
		else if (semantics.Semantic().Setter())
		{
			w.write("set");
		}
		else if (semantics.Semantic().AddOn())
		{
			w.write("addon");
		}
		else if (semantics.Semantic().RemoveOn())
		{
			w.write("removeon");
		}
	}

	static void write_method_semantic(writer& w, MethodSemantics const& semantics)
	{
		auto method = semantics.Method();
		method_signature signature{ method };
		auto type = method.Parent();

		w.write("    .% % % %.%::%(%)\n",
			bind<write_semantic>(semantics),
			method.Flags().Static() ? "static" : "instance",
			signature.return_signature(),
			type.TypeNamespace(),
			type.TypeName(),
			method.Name(),
			bind_list(", ", signature.params()));
	}

	static void write_property(writer& w, Property const& prop)
	{
		w.write("  .property % % %()\n  {\n",
			(prop.Type().CallConvention() && CallingConvention::HasThis) ? "instance" : "static",
			prop.Type(),
			prop.Name());
		w.write_each<write_method_semantic>(prop.MethodSemantic());
		w.write("  }\n");
	}

	static void write_event(writer& w, Event const& evt)
	{
		w.write("  .event class % %\n  {\n", evt.EventType(), evt.Name());
		w.write_each<write_method_semantic>(evt.MethodSemantic());
		w.write("  }\n");
	}

	static void write_delegate(writer& w, TypeDef const& type)
	{
		XLANG_ASSERT(type.Flags().Semantics() == TypeSemantics::Class);
		auto guard{ w.push_generic_params(type.GenericParam()) };

		w.write(".class % % % %%%.%%\n       extends [mscorlib]System.MulticastDelegate\n{\n",
			type.Flags().Visibility(),
			type.Flags().Layout(),
			type.Flags().StringFormat(),
			bind<write_if_true>(type.Flags().WindowsRuntime(), "windowsruntime "),
			bind<write_if_true>(type.Flags().Sealed(), "sealed "),
			type.TypeNamespace(),
			type.TypeName(),
			bind<write_generic_params>(type.GenericParam()));

		// TODO: Guid Custom Attribute
		w.write_each<write_method>(type.MethodList());
		w.write("}\n\n");
	}

	static void write_interface(writer& w, TypeDef const& type)
	{
		XLANG_ASSERT(type.Flags().Semantics() == TypeSemantics::Interface);
		auto guard{ w.push_generic_params(type.GenericParam()) };

		w.write(".class interface % %% % %%.%%\n",
			type.Flags().Visibility(),
			bind<write_if_true>(type.Flags().Abstract(), "abstract "),
			type.Flags().Layout(),
			type.Flags().StringFormat(),
			bind<write_if_true>(type.Flags().WindowsRuntime(), "windowsruntime "),
			type.TypeNamespace(),
			type.TypeName(),
			bind<write_generic_params>(type.GenericParam()));

		separator s{ w, "                  ", "       implements " };
		for (auto&& interface_impl : type.InterfaceImpl())
		{
			s();
			w.write("class %\n", interface_impl.Interface());
		}
		w.write("{\n");

		// TODO: Guid Custom Attribute

		w.write_each<write_method>(type.MethodList());
		w.write_each<write_property>(type.PropertyList());
		w.write_each<write_event>(type.EventList());
		w.write("}\n\n");
	}

	static void write_class(writer& w, TypeDef const& type)
	{
		XLANG_ASSERT(type.Flags().Semantics() == TypeSemantics::Class);
		auto guard{ w.push_generic_params(type.GenericParam()) };

		w.write(".class % %% % %%%.%%\n",
			type.Flags().Visibility(),
			type.Flags().Abstract() ? "abstract " : "",
			type.Flags().Layout(),
			type.Flags().StringFormat(),
			type.Flags().WindowsRuntime() ? "windowsruntime " : "",
			type.Flags().Sealed() ? "sealed " : "",
			type.TypeNamespace(),
			type.TypeName(),
			bind<write_generic_params>(type.GenericParam()));
		w.write("       extends %\n", type.Extends());
		separator s{ w, ",\n                  ", "       implements " };
		for (auto&& interface_impl : type.InterfaceImpl())
		{
			s();
			w.write("%", interface_impl.Interface());
		}
		w.write("\n{\n");
		// TODO: Custom Attributes
		// TODO: .override

		w.write_each<write_method>(type.MethodList());
		w.write_each<write_property>(type.PropertyList());
		w.write_each<write_event>(type.EventList());

		w.write("}\n");
	}

	void write_hex_byte(writer& w, uint8_t value)
	{
		w.write_printf("%X ", value);
	}

	void write_version(writer& w, AssemblyVersion value)
	{
		w.write("%:%:%:%", 
			value.MajorVersion,
			value.MinorVersion,
			value.BuildNumber,
			value.RevisionNumber);
	}

	void write_header(writer& w, database const& db)
	{
		for (auto&& a : db.AssemblyRef)
		{
			w.write(".assembly extern %%\n{\n",
				a.Flags().WindowsRuntime() ? "windowsruntime " : "",
				a.Name());

			auto pkt = a.PublicKeyOrToken();
			if (pkt)
			{
				w.write("  .publickeytoken = (%)\n", bind_each<write_hex_byte>(pkt));
			}

			w.write("  .ver %\n", bind<write_version>(a.Version()));
			w.write("}\n");
		}

		XLANG_ASSERT(db.Assembly.size() == 1);
		auto a = db.Assembly[0];
		w.write(".assembly %%\n{\n",
			a.Flags().WindowsRuntime() ? "windowsruntime " : "",
			a.Name());
		if (a.HashAlgId() != AssemblyHashAlgorithm::None)
		{
			w.write_printf("  .hash algorithm 0x%X\n", a.HashAlgId());
		}
		w.write("  .ver %\n", bind<write_version>(a.Version()));
		w.write("}\n");

		XLANG_ASSERT(db.Module.size() == 1);
		w.write(".module %\n", db.Module[0].Name());

		// TODO: these values come from PE tables not metadata
		w.write(R"(.imagebase 0x00400000
.file alignment 0x00000200
.stackreserve 0x00100000
.subsystem 0x0003
.corflags 0x00000001

)");
	}

    static void run(int const argc, char** argv)
    {
        writer wc;

        try
        {
            process_args(argc, argv);
            cache c{ get_files_to_cache() };

			writer w;
			//w.debug_trace = true;

			XLANG_ASSERT(c.databases().size() == 1);
			write_header(w, c.databases().front());

            for (auto&&[ns, members] : c.namespaces())
            { 
				w.write_each<write_enum>(members.enums);
				w.write_each<write_struct>(members.structs);
				w.write_each<write_delegate>(members.delegates);
				w.write_each<write_interface>(members.interfaces);
				w.write_each<write_class>(members.classes);
            }

			w.flush_to_file(settings.output);
        }
        catch (usage_exception const&)
        {
            print_usage(wc);
			wc.flush_to_console();
		}
        catch (std::exception const& e)
        {
            wc.write(" error: %\n", e.what());
			wc.flush_to_console();
		}
    }
}

int main(int const argc, char** argv)
{
    return rgm::run(argc, argv);
}