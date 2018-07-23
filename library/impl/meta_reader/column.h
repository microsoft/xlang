
namespace xlang::meta::reader
{
    template <typename Row>
    template <typename T>
    auto row_base<Row>::get_list(uint32_t const column) const
    {
        auto const& my_table = get_database().get_table<Row>();
        auto const& target_table = get_database().get_table<T>();

        auto first = target_table.begin() + get_value<uint32_t>(column) - 1;
        auto last = target_table.end();
        if (index() + 1 < my_table.size())
        {
            last = target_table.begin() + my_table[index() + 1].get_value<uint32_t>(column) - 1;
        }
        return std::pair{ first, last };
    }

    template <typename Row>
    template <typename T>
    auto row_base<Row>::get_target_row(uint32_t const column) const
    {
        return get_database().get_table<T>()[get_value<uint32_t>(column) - 1];
    }

    inline auto TypeDef::CustomAttribute() const
    {
        return equal_range(get_database().get_table<reader::CustomAttribute>(), coded_index<HasCustomAttribute>());
    }

    inline auto TypeDef::GenericParam() const
    {
        return equal_range(get_database().GenericParam, coded_index<TypeOrMethodDef>());
    }

    inline auto TypeDef::InterfaceImpl() const
    {
        struct compare
        {
            bool operator()(uint32_t const left, reader::InterfaceImpl const& right) noexcept
            {
                return left < right.get_value<uint32_t>(0) - 1;
            }

            bool operator()(reader::InterfaceImpl const& left, uint32_t const right) noexcept
            {
                return left.get_value<uint32_t>(0) - 1 < right;
            }
        };

        return equal_range(get_database().InterfaceImpl, index(), compare{});
    }

    inline auto TypeDef::FieldList() const
    {
        return get_list<Field>(4);
    }

    inline auto TypeDef::MethodList() const
    {
        return get_list<MethodDef>(5);
    }

    inline auto MethodDef::ParamList() const
    {
        return get_list<Param>(5);
    }

    inline auto InterfaceImpl::Class() const
    {
        return get_target_row<TypeDef>(0);
    }

    inline auto MethodSemantics::Method() const
    {
        return get_target_row<MethodDef>(1);
    }

    inline auto PropertyMap::Parent() const
    {
        return get_target_row<TypeDef>(0);
    }

    inline auto Property::MethodSemantic() const
    {
        return equal_range(get_database().get_table<reader::MethodSemantics>(), coded_index<HasSemantics>());
    }

    inline auto Property::Parent() const
    {
        auto const& map = get_database().get_table<PropertyMap>();
        struct compare
        {
            bool operator()(PropertyMap const& lhs, uint32_t rhs) const noexcept
            {
                return lhs.get_value<uint32_t>(1) < rhs;
            }
            bool operator()(uint32_t lhs, PropertyMap const& rhs) const noexcept
            {
                return lhs < rhs.get_value<uint32_t>(1);
            }
        };
        auto iter = std::lower_bound(map.begin(), map.end(), index() + 1, compare{});
        if (iter.get_value<uint32_t>(1) == index() + 1)
        {
            return iter.Parent();
        }
        else
        {
            return (iter - 1).Parent();
        }
    }

    inline auto PropertyMap::PropertyList() const
    {
        return get_list<Property>(1);
    }

    inline auto TypeDef::PropertyList() const
    {
        auto const& map = get_database().get_table<PropertyMap>();
        auto index = this->index() + 1;
        auto iter = std::find_if(map.begin(), map.end(), [index](PropertyMap const& elem)
        {
            return elem.get_value<uint32_t>(0) == index;
        });
        if (iter == map.end())
        {
            auto const& props = get_database().get_table<Property>();
            return std::pair{ props.end(), props.end() };
        }
        else
        {
            return iter.PropertyList();
        }
    }

    inline auto Field::Constant() const
    {
        auto const range = equal_range(get_database().Constant, coded_index<HasConstant>());
        std::optional<reader::Constant> result;
        if (range.second != range.first)
        {
            WINRT_ASSERT(range.second - range.first == 1);
            result = range.first;
        }
        return result;
    }

    inline auto Constant::ValueBoolean() const
    {
        WINRT_ASSERT(Type() == ConstantType::Boolean);
        return get_blob(2).as<bool>();
    }

    inline auto Constant::ValueChar() const
    {
        WINRT_ASSERT(Type() == ConstantType::Char);
        return get_blob(2).as<char16_t>();
    }

    inline auto Constant::ValueInt8() const
    {
        WINRT_ASSERT(Type() == ConstantType::Int8);
        return get_blob(2).as<int8_t>();
    }

    inline auto Constant::ValueUInt8() const
    {
        WINRT_ASSERT(Type() == ConstantType::UInt8);
        return get_blob(2).as<uint8_t>();
    }

    inline auto Constant::ValueInt16() const
    {
        WINRT_ASSERT(Type() == ConstantType::Int16);
        return get_blob(2).as<int16_t>();
    }

    inline auto Constant::ValueUInt16() const
    {
        WINRT_ASSERT(Type() == ConstantType::UInt16);
        return get_blob(2).as<uint16_t>();
    }

    inline auto Constant::ValueInt32() const
    {
        WINRT_ASSERT(Type() == ConstantType::Int32);
        return get_blob(2).as<int32_t>();
    }

    inline auto Constant::ValueUInt32() const
    {
        WINRT_ASSERT(Type() == ConstantType::UInt32);
        return get_blob(2).as<uint32_t>();
    }

    inline auto Constant::ValueInt64() const
    {
        WINRT_ASSERT(Type() == ConstantType::Int64);
        return get_blob(2).as<int64_t>();
    }

    inline auto Constant::ValueUInt64() const
    {
        WINRT_ASSERT(Type() == ConstantType::UInt64);
        return get_blob(2).as<uint64_t>();
    }

    inline auto Constant::ValueFloat32() const
    {
        WINRT_ASSERT(Type() == ConstantType::Float32);
        return get_blob(2).as<float>();
    }

    inline auto Constant::ValueFloat64() const
    {
        WINRT_ASSERT(Type() == ConstantType::Float64);
        return get_blob(2).as<double>();
    }

    inline auto Constant::ValueString() const
    {
        WINRT_ASSERT(Type() == ConstantType::String);
        return get_blob(2).as_string();
    }

    inline auto Constant::ValueClass() const
    {
        WINRT_ASSERT(Type() == ConstantType::Class);
        WINRT_ASSERT(get_blob(2).as<uint32_t>() == 0);
        return nullptr;
    }

    inline Constant::value_type Constant::Value() const
    {
        switch (Type())
        {
        case ConstantType::Boolean:
            return ValueBoolean();
        case ConstantType::Char:
            return ValueChar();
        case ConstantType::Int8:
            return ValueInt8();
        case ConstantType::UInt8:
            return ValueUInt8();
        case ConstantType::Int16:
            return ValueInt16();
        case ConstantType::UInt16:
            return ValueUInt16();
        case ConstantType::Int32:
            return ValueInt32();
        case ConstantType::UInt32:
            return ValueUInt32();
        case ConstantType::Int64:
            return ValueInt64();
        case ConstantType::UInt64:
            return ValueUInt64();
        case ConstantType::Float32:
            return ValueFloat32();
        case ConstantType::Float64:
            return ValueFloat64();
        case ConstantType::String:
            return ValueString();
        case ConstantType::Class:
            return ValueClass();
        default:
            throw_invalid("Invalid constant type");
        }
    }
}
