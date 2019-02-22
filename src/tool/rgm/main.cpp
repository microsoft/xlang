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
		{ "input", 0, cmd::option::no_max, "<path>", "Windows metadata to include in projection" },
		{ "reference", 0, cmd::option::no_max, "<path>", "Windows metadata to reference from projection" },
	};

	void process_args(int const argc, char** argv)
	{
		cmd::reader args{ argc, argv, options };

		if (!args)
		{
			throw usage_exception{};
		}

		settings.input = args.files("input", database::is_database);
		settings.reference = args.files("reference", database::is_database);
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
		files.insert(files.end(), settings.input.begin(), settings.input.end());
		files.insert(files.end(), settings.reference.begin(), settings.reference.end());
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
				bind<write_if_true>(field.Flags().Literal(), "literal valuetype "),
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


	inline bool operator && (CallingConvention lhs, CallingConvention rhs)
	{
		using T = std::underlying_type_t <CallingConvention>;
		return (static_cast<T>(lhs) & static_cast<T>(rhs)) != 0;
	}

	static void write_interface(writer& w, TypeDef const& type)
	{
		XLANG_ASSERT(type.Flags().Semantics() == TypeSemantics::Interface);
		auto guard{ w.push_generic_params(type.GenericParam()) };

		w.write(".class interface % %% % %%.%\n",
			type.Flags().Visibility(),
			bind<write_if_true>(type.Flags().Abstract(), "abstract "),
			type.Flags().Layout(),
			type.Flags().StringFormat(),
			bind<write_if_true>(type.Flags().WindowsRuntime(), "windowsruntime "),
			type.TypeNamespace(),
			type.TypeName());

		separator s{ w, "                  ", "       implements " };
		for (auto&& interface_impl : type.InterfaceImpl())
		{
			s();
			w.write("%\n", interface_impl.Interface());
		}
		w.write("{\n");

		// TODO: Guid Custom Attribute

		for (auto&& method : type.MethodList())
		{
			method_signature signature{ method };

			w.write(R"(  .method % %% %%%
          % % % (%) % %
  {
  }
)",
				method.Flags().Access(),
				method.Flags().HideBySig() ? "hidebysig " : "",
				method.Flags().Layout(),
				method.Flags().SpecialName() ? "specialname " : "",
				method.Flags().Abstract() ? "abstract " : "",
				method.Flags().Virtual() ? "virtual " : "",
				method.Flags().Static() ? "static" : "instance",
				signature.return_signature(),
				method.Name(),
				bind_list(",\n                  ", signature.params()),
				method.ImplFlags().CodeType(),
				method.ImplFlags().Managed());
		}

		for (auto&& prop : type.PropertyList())
		{
			w.write("  .property % % %()\n  {\n", 
				(prop.Type().CallConvention() && CallingConvention::HasThis) ? "instance" : "static",
				prop.Type(),
				prop.Name());

			for (auto&& method_semantic : prop.MethodSemantic())
			{
				auto method = method_semantic.Method();
				method_signature signature{ method };
				auto semantic = method_semantic.Semantic();
				XLANG_ASSERT(semantic.Getter() || semantic.Setter());

				if (semantic.Getter())
				{
					XLANG_ASSERT(signature.params().size() == 0);

					w.write("    .get % % %.%::%()\n",
						method.Flags().Static() ? "static" : "instance",
						signature.return_signature(),
						type.TypeNamespace(),
						type.TypeName(),
						method.Name());
				}
				else if (semantic.Setter())
				{
					XLANG_ASSERT(signature.params().size() == 1);

					w.write("    .set % % %.%::%(%)\n",
						method.Flags().Static() ? "static" : "instance",
						signature.return_signature(),
						type.TypeNamespace(),
						type.TypeName(),
						method.Name(),
						signature.params()[0].second->Type());
				}
			}

			w.write("  }\n");
		}

		w.write("}\n\n");
	}

    static void run(int const argc, char** argv)
    {
        writer w;
		w.debug_trace = true;

        try
        {
            process_args(argc, argv);
            cache c{ get_files_to_cache() };

            for (auto&&[ns, members] : c.namespaces())
            { 
                if (ns.compare(0, 18, "Windows.Foundation") == 0)
                    continue;

				//w.write_each<write_enum>(members.enums);
				//w.write_each<write_struct>(members.structs);
				w.write_each<write_interface>(members.interfaces);
            }
        }
        catch (usage_exception const&)
        {
            print_usage(w);
			w.flush_to_console();
		}
        catch (std::exception const& e)
        {
            w.write(" error: %\n", e.what());
			w.flush_to_console();
		}

		system("pause");
    }
}

int main(int const argc, char** argv)
{
    return rgm::run(argc, argv);
}