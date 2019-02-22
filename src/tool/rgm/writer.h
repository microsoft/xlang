namespace rgm
{
	using namespace xlang;
	using namespace xlang::text;
	using namespace xlang::meta::reader;

	struct writer : xlang::text::writer_base<writer>
	{
		using writer_base<writer>::write;

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
			default:
				throw_invalid("invalid TypeLayout");
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

		void write(Constant const& value)
		{
			switch (value.Type())
			{
			case ConstantType::Int32:
				write_printf("int(%d)", value.ValueInt32());
				break;
			case ConstantType::UInt32:
				write_printf("uint(%#0x)", value.ValueUInt32());
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
			default:
				throw_invalid("ElementType");
			}
		}

		void write(coded_index<ResolutionScope> const& scope)
		{
			switch (scope.type())
			{
			case ResolutionScope::Module:
				break;
			default:
				throw_invalid("not impl");
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
			default:
				throw_invalid("TypeDefOrRef");
			}
		}

		void write(GenericTypeIndex const&)
		{
			throw_invalid("GenericTypeIndex");
		}

		void write(GenericTypeInstSig const&)
		{
			throw_invalid("GenericTypeInstSig");
		}

		void write(TypeSig const& signature)
		{
			call(signature.Type(),
				[&](auto&& type)
			{
				write(type);
			});
		}
	};
}