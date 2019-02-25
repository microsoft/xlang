namespace rgm
{
	using namespace xlang;
	using namespace xlang::text;
	using namespace xlang::meta::reader;


	struct writer : xlang::text::writer_base<writer>
	{
		using writer_base<writer>::write;

#pragma region generic param handling
		std::vector<std::vector<std::string>> generic_param_stack;

		struct generic_param_guard
		{
			explicit generic_param_guard(writer* arg = nullptr)
				: owner(arg)
			{}

			~generic_param_guard()
			{
				if (owner)
				{
					owner->generic_param_stack.pop_back();
				}
			}

			generic_param_guard(generic_param_guard && other)
				: owner(other.owner)
			{
				owner = nullptr;
			}

			generic_param_guard& operator=(generic_param_guard&& other)
			{
				owner = std::exchange(other.owner, nullptr);
				return *this;
			}

			generic_param_guard& operator=(generic_param_guard const&) = delete;
			writer* owner;
		};

		[[nodiscard]] auto push_generic_params(std::pair<GenericParam, GenericParam>&& params)
		{
			if (empty(params))
			{
				return generic_param_guard{ nullptr };
			}

			std::vector<std::string> names;

			for (auto&& param : params)
			{
				names.push_back(std::string{ param.Name() });
			}

			generic_param_stack.push_back(std::move(names));
			return generic_param_guard{ this };
		}

		[[nodiscard]] auto push_generic_params(GenericTypeInstSig const& signature)
		{
			std::vector<std::string> names;

			for (auto&& arg : signature.GenericArgs())
			{
				names.push_back(write_temp("%", arg));
			}

			generic_param_stack.push_back(std::move(names));
			return generic_param_guard{ this };
		}

		[[nodiscard]] auto push_generic_params(std::vector<std::string> const& type_arguments)
		{
			if (type_arguments.size() == 0)
			{
				return generic_param_guard{ nullptr };
			}

			std::vector<std::string> names;

			for (auto&& type_argument : type_arguments)
			{
				names.push_back(type_argument);
			}

			generic_param_stack.push_back(std::move(names));
			return generic_param_guard{ this };
		}

#pragma endregion
		void write(MemberAccess access)
		{
			switch (access)
			{
			case MemberAccess::Public:
				write("public");
				break;
			case MemberAccess::Private:
				write("private");
				break;
			default:
				throw_invalid("invalid MemberAccess");
			}
		}

		void write(TypeVisibility visibility)
		{
			switch (visibility)
			{
			case TypeVisibility::Public:
				write("public");
				break;
			case TypeVisibility::NotPublic:
				write("private");
				break;
			default:
				throw_invalid("invalid TypeVisibility");
			}
		}

		void write(TypeLayout layout)
		{
			switch (layout)
			{
			case TypeLayout::AutoLayout:
				write("auto");
				break;
			case TypeLayout::SequentialLayout:
				write("sequential");
				break;
			default:
				throw_invalid("invalid TypeLayout");
			}
		}

		void write(VtableLayout layout)
		{
			switch (layout)
			{
			case VtableLayout::ReuseSlot:
				break;
			case VtableLayout::NewSlot:
				write("newslot");
				break;
			default:
				throw_invalid("invalid VtableLayout");
			}
		}

		void write(StringFormat format)
		{
			switch (format)
			{
			case StringFormat::AnsiClass:
				write("ansi");
				break;
			default:
				throw_invalid("invalid StringFormat");
			}
		}

		void write(Managed managed)
		{
			switch (managed)
			{
			case Managed::Managed:
				write("managed");
				break;
			default:
				throw_invalid("invalid Managed");
			}
		}

		void write(CodeType code_type)
		{
			switch (code_type)
			{
			case CodeType::IL:
				write("cil");
				break;
			case CodeType::Runtime:
				write("runtime");
				break;
			default:
				throw_invalid("invalid CodeType");
			}
		}

		void write(Constant const& value)
		{
			switch (value.Type())
			{
			case ConstantType::Int32:
				write_printf("int32(0x%x)", value.ValueInt32());
				break;
			case ConstantType::UInt32:
				write_printf("uint32(0x%x)", value.ValueUInt32());
				break;
			default:
				throw_invalid("Constant");
			}
		}

		void write(ElementType type)
		{
			switch (type)
			{
			case ElementType::Boolean:
				write("bool"); break;
			case ElementType::Char:
				write("char"); break;
			case ElementType::I1:
				write("int8"); break;
			case ElementType::U1:
				write("uint8"); break;
			case ElementType::I2:
				write("int16"); break;
			case ElementType::U2:
				write("uint16"); break;
			case ElementType::I4:
				write("int32"); break;
			case ElementType::U4:
				write("uint32"); break;
			case ElementType::I8:
				write("int64"); break;
			case ElementType::U8:
				write("uint64"); break;
			case ElementType::R4:
				write("float32"); break;
			case ElementType::R8:
				write("float64"); break;
			case ElementType::String:
				write("string"); break;
			case ElementType::Object:
				write("object"); break;
			case ElementType::I:
				write("native int"); break;
			case ElementType::End:
			case ElementType::Void:
				write("void"); break;
			default:
				throw_invalid("ElementType");
			}
		}

		void write(AssemblyRef ref)
		{
			write("[%]", ref.Name());
		}

		void write(coded_index<ResolutionScope> const& scope)
		{
			switch (scope.type())
			{
			case ResolutionScope::Module:
				break;
			case ResolutionScope::AssemblyRef:
				write(scope.AssemblyRef());
				break;
			default:
				throw_invalid("coded_index<ResolutionScope>");
			}
		}

		void write(TypeRef const& type)
		{
			write("%%.%", type.ResolutionScope(), type.TypeNamespace(), type.TypeName());
		}

		void write(TypeDef const& type)
		{
			write("%.%", type.TypeNamespace(), type.TypeName());
		}

		void write(coded_index<TypeDefOrRef> const& type)
		{
			switch (type.type())
			{
			case TypeDefOrRef::TypeDef:
				write(type.TypeDef());
				break;
			case TypeDefOrRef::TypeRef:
				write(type.TypeRef());
				break;
			case TypeDefOrRef::TypeSpec:
				write(type.TypeSpec().Signature().GenericTypeInst());
				break;
			default:
				throw_invalid("TypeDefOrRef");
			}
		}

		void write(GenericTypeIndex const& var)
		{
			write("!%", generic_param_stack.back()[var.index]);
		}

		void write(GenericTypeInstSig const& type)
		{
			write("%<%>", type.GenericType(), bind_list(", ", type.GenericArgs()));
		}

		void write(TypeSig const& signature)
		{
			switch (signature.element_type())
			{
			case ElementType::ValueType:
				write("valuetype ");
				break;
			case ElementType::Class:
				write("class ");
				break;
			case ElementType::GenericInst:
			{
				auto sig = std::get<GenericTypeInstSig>(signature.Type());
				switch (sig.ClassOrValueType())
				{
				case ElementType::ValueType:
					write("valuetype ");
					break;
				case ElementType::Class:
					write("class ");
					break;
				}
			}
				break;
			}

			call(signature.Type(),
				[&](auto&& type)
			{
				write(type);
			});

			if (signature.is_szarray())
			{
				write("[]");
			}
		}

		void write_param_name(std::string_view const& name)
		{
			if (name == "value" || name == "object" || name == "method")
			{
				write("'%'", name);
			}
			else
			{
				write(name);
			}
		}

		void write(std::pair<Param, ParamSig const*> const& param)
		{
			// make sure param is not marked both in and out
			XLANG_ASSERT(!(param.first.Flags().In() && param.first.Flags().Out()));

			write("%% % ", 
				param.first.Flags().In() ? "[in]" : "",
				param.first.Flags().Out() ? "[out]" : "",
				param.second->Type());
			write_param_name(param.first.Name());
		}

		void write(GenericParam const& param)
		{
			write(param.Name());
		}

		void write(RetTypeSig const& signature)
		{
			write(signature.Type());
		}

		void write(PropertySig const& signature)
		{
			write(signature.Type());
		}
	};

	struct separator
	{
		writer& w;
		std::string_view _separator{ ", " };
		std::string_view _prefix{ };
		bool first{ true };

		void operator()()
		{
			if (first)
			{
				first = false;
				w.write(_prefix);
			}
			else
			{
				w.write(_separator);
			}
		}
	};
}