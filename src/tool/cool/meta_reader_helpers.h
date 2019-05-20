#pragma once

namespace xlang::meta::reader
{
	template <typename T>
	bool has_attribute(T const& row, std::string_view const& type_namespace, std::string_view const& type_name)
	{
		return static_cast<bool>(get_attribute(row, type_namespace, type_name));
	}

	//template <typename T>
	//auto get_attribute_value(CustomAttribute const& attribute, uint32_t const arg)
	//{
	//	return std::get<T>(std::get<ElemSig>(attribute.Value().FixedArgs()[arg].value).value);
	//}

    bool is_exclusive_to(TypeDef const& type)
    {
        return get_category(type) == category::interface_type && has_attribute(type, "Windows.Foundation.Metadata"sv, "ExclusiveToAttribute"sv);
    }

    bool is_flags_enum(TypeDef const& type)
    {
        return get_category(type) == category::enum_type && has_attribute(type, "System"sv, "FlagsAttribute"sv);
    }

    bool is_api_contract_type(TypeDef const& type)
    {
        return get_category(type) == category::struct_type && has_attribute(type, "Windows.Foundation.Metadata"sv, "ApiContractAttribute"sv);
    }

    bool is_attribute_type(TypeDef const& type)
    {
        return get_category(type) == category::class_type && extends_type(type, "System"sv, "Attribute"sv);
    }

	bool is_ptype(TypeDef const& type)
	{
		return distance(type.GenericParam()) > 0;
	}

	bool is_static(TypeDef const& type)
	{
		return get_category(type) == category::class_type && type.Flags().Abstract();
	}

    bool is_constructor(MethodDef const& method)
    {
        return method.Flags().RTSpecialName() && method.Name() == ".ctor";
    }

    bool is_special(MethodDef const& method)
    {
        return method.SpecialName() || method.Flags().RTSpecialName();
    }

	bool is_static(MethodDef const& method)
	{
		return method.Flags().Static();
	}

    auto get_delegate_invoke(TypeDef const& type)
    {
        XLANG_ASSERT(get_category(type) == category::delegate_type);

        for (auto&& method : type.MethodList())
        {
            if (method.SpecialName() && (method.Name() == "Invoke"))
            {
                return method;
            }
        }

        throw_invalid("Invoke method not found");
    }

    enum class fundamental_type
    {
        Boolean,
        Char,
        Int8,
        UInt8,
        Int16,
        UInt16,
        Int32,
        UInt32,
        Int64,
        UInt64,
        Float,
        Double,
        String,
    };

    struct generic_type_instance;
    struct object_type {};
    struct guid_type {};
    using type_definition = TypeDef;
    using generic_type_index = GenericTypeIndex;

    using type_semantics = std::variant<
        fundamental_type,
        object_type,
        guid_type,
        type_definition,
        generic_type_instance,
        generic_type_index>;

    struct generic_type_instance
    {
        type_definition generic_type;
        std::vector<type_semantics> generic_args{};
    };

    type_semantics get_type_semantics(TypeSig const& signature);

    type_semantics get_type_semantics(GenericTypeInstSig const& type)
    {
        auto generic_type_helper = [&type]()
        {
            switch (type.GenericType().type())
            {
            case TypeDefOrRef::TypeDef:
                return type.GenericType().TypeDef();
            case TypeDefOrRef::TypeRef:
                return find_required(type.GenericType().TypeRef());
            }

            throw_invalid("invalid TypeDefOrRef value for GenericTypeInstSig.GenericType");
        };

        auto gti = generic_type_instance{ generic_type_helper() };

        for (auto&& arg : type.GenericArgs())
        {
            gti.generic_args.push_back(get_type_semantics(arg));
        }

        return gti;
    }

    type_semantics get_type_semantics(coded_index<TypeDefOrRef> const& type)
    {
        switch (type.type())
        {
        case TypeDefOrRef::TypeDef:
            return type.TypeDef();
        case TypeDefOrRef::TypeRef:
        {
            auto type_ref = type.TypeRef();
            if (type_ref.TypeName() == "Guid" && type_ref.TypeNamespace() == "System")
            {
                return guid_type{};
            }

            if (type_ref.TypeName() == "Object" && type_ref.TypeNamespace() == "System")
            {
                return object_type{};
            }

            if (type_ref.TypeName() == "Type" && type_ref.TypeNamespace() == "System")
            {
                throw_invalid("System.Type typeref not implemented");
            }

            return find_required(type_ref);
        }
        case TypeDefOrRef::TypeSpec:
            return get_type_semantics(type.TypeSpec().Signature().GenericTypeInst());
        }

        throw_invalid("TypeDefOrRef not supported");
    }

    namespace impl
    {
        template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
        template<class... Ts> overloaded(Ts...)->overloaded<Ts...>;
    }

    type_semantics get_type_semantics(TypeSig const& signature)
    {
        return std::visit(
            impl::overloaded{
            [](ElementType type) -> type_semantics
        {
            switch (type)
            {
            case ElementType::Boolean:
                return fundamental_type::Boolean;
            case ElementType::Char:
                return fundamental_type::Char;
            case ElementType::I1:
                return fundamental_type::Int8;
            case ElementType::U1:
                return fundamental_type::UInt8;
            case ElementType::I2:
                return fundamental_type::Int16;
            case ElementType::U2:
                return fundamental_type::UInt16;
            case ElementType::I4:
                return fundamental_type::Int32;
            case ElementType::U4:
                return fundamental_type::UInt32;
            case ElementType::I8:
                return fundamental_type::Int64;
            case ElementType::U8:
                return fundamental_type::UInt64;
            case ElementType::R4:
                return fundamental_type::Float;
            case ElementType::R8:
                return fundamental_type::Double;
            case ElementType::String:
                return fundamental_type::String;
            case ElementType::Object:
                return object_type{};
            }
            throw_invalid("element type not supported");
        },
            [](coded_index<TypeDefOrRef> type) -> type_semantics
        {
            return get_type_semantics(type);
        },
            [](GenericTypeIndex var) -> type_semantics { return generic_type_index{ var.index }; },
            [](GenericTypeInstSig sig) -> type_semantics { return get_type_semantics(sig); },
            [](GenericMethodTypeIndex) -> type_semantics { throw_invalid("Generic methods not supported"); }
            }, signature.Type());
    }

	TypeDef get_typedef(type_semantics const& semantics)
	{
		return std::visit(
			impl::overloaded{
				[](type_definition type) { return type; },
				[](generic_type_instance type_instance) { return type_instance.generic_type; },
				[](auto) -> TypeDef { throw_invalid("type doesn't contain typedef"); }
			}, semantics);
	};

	TypeDef get_typedef(coded_index<TypeDefOrRef> const& type)
	{
		return get_typedef(get_type_semantics(type));
	};

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

            for (uint32_t i{}; i != size(m_method.Params()); ++i)
            {
                m_params.emplace_back(params.first + i, &m_method.Params().first[i]);
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

        auto return_param_name(std::string_view default_name = "__return_value__") const
        {
            if (m_return)
            {
                return m_return.Name();
            }
            else
            {
                return default_name;
            }
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

    enum class param_category
    {
        in,
        out,
        pass_array,
        fill_array,
        receive_array,
    };

    auto get_param_category(method_signature::param_t const& param)
    {
        if (param.second->Type().is_szarray())
        {
            if (param.first.Flags().In())
            {
                return param_category::pass_array;
            }
            else if (param.second->ByRef())
            {
                XLANG_ASSERT(param.first.Flags().Out());
                return param_category::receive_array;
            }
            else
            {
                XLANG_ASSERT(param.first.Flags().Out());
                return param_category::fill_array;
            }
        }
        else
        {
            if (param.first.Flags().In())
            {
                XLANG_ASSERT(!param.first.Flags().Out());
                return param_category::in;
            }
            else
            {
                XLANG_ASSERT(param.first.Flags().Out());
                return param_category::out;
            }
        }
    }

    auto get_property_methods(Property const& prop)
    {
        MethodDef get_method{}, set_method{};

        for (auto&& method_semantic : prop.MethodSemantic())
        {
            auto semantic = method_semantic.Semantic();

            if (semantic.Getter())
            {
                get_method = method_semantic.Method();
            }
            else if (semantic.Setter())
            {
                set_method = method_semantic.Method();
            }
            else
            {
                throw_invalid("Properties can only have get and set methods");
            }
        }

        XLANG_ASSERT(get_method);

        if (set_method)
        {
            XLANG_ASSERT(get_method.Flags().Static() == set_method.Flags().Static());
        }

        return std::make_tuple(get_method, set_method);
    }

	auto get_event_methods(Event const& evt)
	{
		MethodDef add_method{}, remove_method{};

		for (auto&& method_semantic : evt.MethodSemantic())
		{
			auto semantic = method_semantic.Semantic();

			if (semantic.AddOn())
			{
				add_method = method_semantic.Method();
			}
			else if (semantic.RemoveOn())
			{
				remove_method = method_semantic.Method();
			}
			else
			{
				throw_invalid("Events can only have add and remove methods");
			}
		}

		XLANG_ASSERT(add_method);
		XLANG_ASSERT(remove_method);
		XLANG_ASSERT(add_method.Flags().Static() == remove_method.Flags().Static());

		return std::make_tuple(add_method, remove_method);
	}

	struct activation_factory
	{
		TypeDef type;
	};

	struct static_factory
	{
		TypeDef type;
	};

	// TODO: composable factory

	using factory_info = std::variant<activation_factory, static_factory>;

	auto get_factories(TypeDef const& type)
	{
		auto get_system_type = [&](FixedArgSig const& fixed_arg, bool optional = false) -> TypeDef
		{
			if (auto type_param = std::get_if<ElemSig::SystemType>(&std::get<ElemSig>(fixed_arg.value).value))
			{
				return type.get_cache().find_required(type_param->name);
			}
			
			if (optional)
			{
				return {};
			}

			throw_invalid("Invalid factory argument");
		};

		std::vector<factory_info> result;

		for (auto&& attribute : type.CustomAttribute())
		{
			auto attribute_name = attribute.TypeNamespaceAndName();

			if (attribute_name.first != "Windows.Foundation.Metadata")
			{
				continue;
			}

			auto fixed_args = attribute.Value().FixedArgs();

			if (attribute_name.second == "ActivatableAttribute")
			{
				activation_factory info{ get_system_type(fixed_args[0], true) };
				result.push_back(std::move(info));
			}
			else if (attribute_name.second == "StaticAttribute")
			{
				static_factory info{ get_system_type(fixed_args[0], true) };
				XLANG_ASSERT((bool)info.type);
				result.push_back(std::move(info));
			}
			else if (attribute_name.second == "ComposableAttribute")
			{
				throw_invalid("ComposableAttribute not implemented");
				//info.type = get_system_type(fixed_args[0]);
				//info.composable = true;

				//auto compositionType = std::get<ElemSig::EnumValue>(std::get<ElemSig>(fixed_args[1].value).value);
				//info.visible = std::get<int32_t>(compositionType.value) == 2;
			}
		}

		return std::move(result);
	}

	/*static auto get_default_interface(TypeDef const& type)
	{
		auto impls = type.InterfaceImpl();

		for (auto&& impl : impls)
		{
			if (has_attribute(impl, "Windows.Foundation.Metadata", "DefaultAttribute"))
			{
				return impl.Interface();
			}
		}

		if (!empty(impls))
		{
			throw_invalid("Type '", type.TypeNamespace(), ".", type.TypeName(), "' does not have a default interface");
		}

		return {};
	}
*/
}