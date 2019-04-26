#pragma once

namespace xlang::meta::reader
{
    inline uint32_t uncompress_unsigned(byte_view& cursor)
    {
        auto data = cursor.begin();
        uint32_t value;
        uint32_t length;
        if ((*data & 0x80) == 0x00)
        {
            length = 1;
            value = *data;
        }
        else if ((*data & 0xc0) == 0x80)
        {
            length = 2;
            value = (*data++ & 0x3f) << 8;
            value |= *data;
        }
        else if ((*data & 0xe0) == 0xc0)
        {
            length = 4;
            value = (*data++ & 0x1f) << 24;
            value |= *data++ << 16;
            value |= *data++ << 8;
            value |= *data;
        }
        else
        {
            throw_invalid("Invalid compressed integer in blob");
        }
        cursor = cursor.seek(length);
        return value;
    }

    template <typename T>
    T uncompress_enum(byte_view& cursor)
    {
        static_assert(std::is_enum_v<T>);
        static_assert(std::is_unsigned_v<std::underlying_type_t<T>>);
        return static_cast<T>(uncompress_unsigned(cursor));
    }

    template <typename T>
    T read(byte_view& cursor)
    {
        auto const& result = cursor.as<T>();
        cursor = cursor.seek(sizeof(T));
        return result;
    }

    template <>
    inline std::string_view read<std::string_view>(byte_view& cursor)
    {
        uint32_t const length = uncompress_unsigned(cursor);
        std::string_view result{ reinterpret_cast<const char*>(cursor.begin()), length };
        cursor = cursor.seek(length);
        return result;
    }

    struct CustomModSig;
    struct FieldSig;
    struct GenericTypeInstSig;
    struct MethodDefSig;
    struct ParamSig;
    struct PropertySig;
    struct RetTypeSig;
    struct TypeSig;
    struct TypeSpecSig;

    struct CustomModSig
    {
        CustomModSig(table_base const* table, byte_view& data)
            : m_cmod(uncompress_enum<ElementType>(data))
            , m_type(table, uncompress_unsigned(data))
        {
            XLANG_ASSERT(m_cmod == ElementType::CModReqd || m_cmod == ElementType::CModOpt);
        }

        explicit CustomModSig(ElementType cmod, coded_index<TypeDefOrRef> const& type)
            : m_cmod(cmod)
            , m_type(type)
        {
            XLANG_ASSERT((m_cmod == ElementType::CModOpt) || (m_cmod == ElementType::CModReqd));
        }

        ElementType CustomMod() const noexcept
        {
            return m_cmod;
        }

        coded_index<TypeDefOrRef> Type() const noexcept
        {
            return m_type;
        }

    private:
        ElementType m_cmod;
        coded_index<TypeDefOrRef> m_type;
    };

    struct GenericTypeInstSig
    {
        GenericTypeInstSig(table_base const* table, byte_view& data);

        GenericTypeInstSig(ElementType class_or_value, coded_index<TypeDefOrRef> type, std::vector<TypeSig> const& generic_args)
            : m_class_or_value(class_or_value)
            , m_type(type)
            , m_generic_args(generic_args)
        {
            XLANG_ASSERT((m_class_or_value == ElementType::Class) || (m_class_or_value == ElementType::ValueType));
        }

        GenericTypeInstSig(ElementType class_or_value, coded_index<TypeDefOrRef> type, std::vector<TypeSig>&& generic_args)
            : m_class_or_value(class_or_value)
            , m_type(type)
            , m_generic_args(std::move(generic_args))
        {
            XLANG_ASSERT((m_class_or_value == ElementType::Class) || (m_class_or_value == ElementType::ValueType));
        }

        ElementType ClassOrValueType() const noexcept
        {
            return m_class_or_value;
        }

        coded_index<TypeDefOrRef> GenericType() const noexcept
        {
            return m_type;
        }

        uint32_t GenericArgCount() const noexcept
        {
            return static_cast<uint32_t>(m_generic_args.size());
        }

        auto GenericArgs() const noexcept
        {
            return std::pair{ m_generic_args.cbegin(), m_generic_args.cend() };
        }

    private:
        ElementType m_class_or_value;
        coded_index<TypeDefOrRef> m_type;
        std::vector<TypeSig> m_generic_args;
    };

    inline std::vector<CustomModSig> parse_cmods(table_base const* table, byte_view& data)
    {
        std::vector<CustomModSig> result;
        auto cursor = data;

        for (auto element_type = uncompress_enum<ElementType>(cursor);
            element_type == ElementType::CModOpt || element_type == ElementType::CModReqd;
            element_type = uncompress_enum<ElementType>(cursor))
        {
            result.emplace_back(table, data);
            cursor = data;
        }
        return result;
    }

    inline bool parse_szarray(table_base const*, byte_view& data)
    {
        auto cursor = data;
        if (uncompress_enum<ElementType>(cursor) == ElementType::SZArray)
        {
            data = cursor;
            return true;
        }
        return false;
    }

    struct GenericTypeIndex
    {
        uint32_t index;
    };

    struct GenericMethodTypeIndex
    {
        uint32_t index;
    };

    struct TypeSig
    {
        using value_type = std::variant<ElementType, coded_index<TypeDefOrRef>, GenericTypeIndex, GenericTypeInstSig, GenericMethodTypeIndex>;

        TypeSig(table_base const* table, byte_view& data)
            : m_is_szarray(parse_szarray(table, data))
            , m_cmod(parse_cmods(table, data))
            , m_element_type(parse_element_type(data))
            , m_type(ParseType(table, data))
        {}

        explicit TypeSig(ElementType type)
            : m_element_type{ type }
            , m_type{ type }
        {}

        explicit TypeSig(ElementType element_type, coded_index<TypeDefOrRef> const& type_ref)
            : m_element_type{ element_type }
            , m_type{ type_ref }
        {}

        value_type const& Type() const noexcept
        {
            return m_type;
        }

        ElementType element_type() const noexcept
        {
            return m_element_type;
        }

        bool is_szarray() const noexcept
        {
            return m_is_szarray;
        }

        void is_szarray(bool value) noexcept
        {
            m_is_szarray = value;
        }

    private:
        static ElementType parse_element_type(byte_view& data)
        {
            auto cursor = data;
            return uncompress_enum<ElementType>(cursor);
        }

        static value_type ParseType(table_base const* table, byte_view& data);
        bool m_is_szarray{ false };
        std::vector<CustomModSig> m_cmod;
        ElementType m_element_type;
        value_type m_type;
    };

    inline bool is_by_ref(byte_view& data)
    {
        auto cursor = data;
        auto element_type = uncompress_enum<ElementType>(cursor);
        if (element_type == ElementType::ByRef)
        {
            data = cursor;
            return true;
        }
        else
        {
            XLANG_ASSERT(element_type != ElementType::TypedByRef);
            return false;
        }
    }

    struct ParamSig
    {
        ParamSig(table_base const* table, byte_view& data)
            : m_cmod(parse_cmods(table, data))
            , m_byref(is_by_ref(data))
            , m_type(table, data)
        {
        }

        explicit ParamSig(TypeSig const& type)
            : m_type{ type }
        {}

        auto CustomMod() const noexcept
        {
            return std::pair{ m_cmod.cbegin(), m_cmod.cend() };
        }

        bool ByRef() const noexcept
        {
            return m_byref;
        }

        TypeSig const& Type() const noexcept
        {
            return m_type;
        }

    private:
        std::vector<CustomModSig> m_cmod;
        bool m_byref;
        TypeSig m_type;
    };

    struct RetTypeSig
    {
        RetTypeSig(table_base const* table, byte_view& data)
            : m_cmod(parse_cmods(table, data))
            , m_byref(is_by_ref(data))
        {
            auto cursor = data;
            auto element_type = uncompress_enum<ElementType>(cursor);
            if (element_type == ElementType::Void)
            {
                data = cursor;
            }
            else
            {
                m_type.emplace(table, data);
            }
        }

        explicit RetTypeSig(std::optional<TypeSig> const& type)
            : m_type{ type }
        {}

        auto CustomMod() const noexcept
        {
            return std::pair{ m_cmod.cbegin(), m_cmod.cend() };
        }

        bool ByRef() const noexcept
        {
            return m_byref;
        }

        TypeSig const& Type() const noexcept
        {
            return *m_type;
        }

        explicit operator bool() const noexcept
        {
            return m_type.has_value();
        }

    private:
        std::vector<CustomModSig> m_cmod;
        bool m_byref;
        std::optional<TypeSig> m_type;
    };

    struct MethodDefSig
    {
        MethodDefSig(table_base const* table, byte_view& data)
            : m_calling_convention(uncompress_enum<CallingConvention>(data))
            , m_generic_param_count(enum_mask(m_calling_convention, CallingConvention::Generic) == CallingConvention::Generic ? uncompress_unsigned(data) : 0)
            , m_param_count(uncompress_unsigned(data))
            , m_ret_type(table, data)
        {
            if (m_param_count > data.size())
            {
                throw_invalid("Invalid blob array size");
            }
            m_params.reserve(m_param_count);
            for (uint32_t count = 0; count < m_param_count; ++count)
            {
                m_params.emplace_back(table, data);
            }
        }

        explicit MethodDefSig(RetTypeSig const& ret_type, std::vector<ParamSig> const& param_type)
            : m_ret_type(ret_type), m_params(param_type)
        {}

        explicit MethodDefSig(RetTypeSig&& type, std::vector<ParamSig> && param_type)
            : m_ret_type(std::move(type)), m_params(std::move(param_type))
        {}

        CallingConvention CallConvention() const noexcept
        {
            return m_calling_convention;
        }

        uint32_t GenericParamCount() const noexcept
        {
            return m_generic_param_count;
        }

        RetTypeSig const& ReturnType() const noexcept
        {
            return m_ret_type;
        }

        auto Params() const noexcept
        {
            return std::pair{ m_params.cbegin(), m_params.cend() };
        }

    private:
        CallingConvention m_calling_convention;
        uint32_t m_generic_param_count;
        uint32_t m_param_count;
        RetTypeSig m_ret_type;
        std::vector<ParamSig> m_params;
    };

    struct FieldSig
    {
        FieldSig(table_base const* table, byte_view& data)
            : m_calling_convention(check_convention(data))
            , m_cmod(parse_cmods(table, data))
            , m_type(table, data)
        {}

        explicit FieldSig(TypeSig const& type)
            : m_type(type)
        {}

        explicit FieldSig(TypeSig&& type)
            : m_type(std::move(type))
        {}

        auto CustomMod() const noexcept
        {
            return std::pair{ m_cmod.cbegin(), m_cmod.cend() };
        }

        TypeSig const& Type() const noexcept
        {
            return m_type;
        }

        void is_static(bool value) noexcept
        {
            if (value)
            {
                m_calling_convention = static_cast<CallingConvention>(static_cast<uint8_t>(m_calling_convention) | static_cast<uint8_t>(CallingConvention::HasThis));
            }
            else
            {
                m_calling_convention = enum_mask(m_calling_convention, CallingConvention::Mask);
            }
        }

        CallingConvention get_CallingConvention() const noexcept
        {
            return m_calling_convention;
        }

    private:
        static CallingConvention check_convention(byte_view& data)
        {
            auto conv = read<CallingConvention>(data);
            if (enum_mask(conv, CallingConvention::Field) != CallingConvention::Field)
            {
                throw_invalid("Invalid calling convention for field blob");
            }
            return conv;
        }
        CallingConvention m_calling_convention{ CallingConvention::Field };
        std::vector<CustomModSig> m_cmod;
        TypeSig m_type;
    };

    struct PropertySig
    {
        PropertySig(table_base const* table, byte_view& data)
            : m_calling_convention(check_convention(data))
            , m_param_count(uncompress_unsigned(data))
            , m_cmod(parse_cmods(table, data))
            , m_type(table, data)
        {
            if (m_param_count > data.size())
            {
                throw_invalid("Invalid blob array size");
            }
            m_params.reserve(m_param_count);
            for (uint32_t count = 0; count < m_param_count; ++count)
            {
                m_params.emplace_back(table, data);
            }
        }

        TypeSig const& Type() const noexcept
        {
            return m_type;
        }

        CallingConvention CallConvention() const noexcept
        {
            return m_calling_convention;
        }

        auto CustomMod() const noexcept
        {
            return std::pair{ m_cmod.cbegin(), m_cmod.cend() };
        }

        uint32_t ParamCount() const noexcept
        {
            return static_cast<uint32_t>(m_params.size());
        }

        auto Params() const noexcept
        {
            return std::pair{ m_params.cbegin(), m_params.cend() };
        }

    private:
        static CallingConvention check_convention(byte_view& data)
        {
            auto conv = read<CallingConvention>(data);
            if (enum_mask(conv, CallingConvention::Property) != CallingConvention::Property)
            {
                throw_invalid("Invalid calling convention for property blob");
            }
            return conv;
        }
        CallingConvention m_calling_convention;
        uint32_t m_param_count;
        std::vector<CustomModSig> m_cmod;
        TypeSig m_type;
        std::vector<ParamSig> m_params;
    };

    struct TypeSpecSig
    {
        TypeSpecSig(table_base const* table, byte_view& data)
            : m_type(ParseType(table, data))
        {
        }

        GenericTypeInstSig const& GenericTypeInst() const noexcept
        {
            return m_type;
        }

    private:
        static GenericTypeInstSig ParseType(table_base const* table, byte_view& data)
        {
            [[maybe_unused]] auto element_type = uncompress_enum<ElementType>(data);
            XLANG_ASSERT(element_type == ElementType::GenericInst);
            return { table, data };
        }
        GenericTypeInstSig m_type;
    };

    inline GenericTypeInstSig::GenericTypeInstSig(table_base const* table, byte_view& data)
        : m_class_or_value(uncompress_enum<ElementType>(data))
        , m_type(table, uncompress_unsigned(data))
    {
        if (!(m_class_or_value == ElementType::Class || m_class_or_value == ElementType::ValueType))
        {
            throw_invalid("Generic type instantiation signatures must begin with either ELEMENT_TYPE_CLASS or ELEMENT_TYPE_VALUE");
        }

        uint32_t const generic_arg_count = uncompress_unsigned(data);

        if (generic_arg_count > data.size())
        {
            throw_invalid("Invalid blob array size");
        }
        m_generic_args.reserve(generic_arg_count);
        for (uint32_t arg = 0; arg < generic_arg_count; ++arg)
        {
            m_generic_args.emplace_back(table, data);
        }
    }

    inline TypeSig::value_type TypeSig::ParseType(table_base const* table, byte_view& data)
    {
        auto element_type = uncompress_enum<ElementType>(data);
        switch (element_type)
        {
        case ElementType::Boolean:
        case ElementType::Char:
        case ElementType::I1:
        case ElementType::U1:
        case ElementType::I2:
        case ElementType::U2:
        case ElementType::I4:
        case ElementType::U4:
        case ElementType::I8:
        case ElementType::U8:
        case ElementType::R4:
        case ElementType::R8:
        case ElementType::String:
        case ElementType::Object:
        case ElementType::U:
        case ElementType::I:
            return element_type;
            break;

        case ElementType::Class:
        case ElementType::ValueType:
            return coded_index<TypeDefOrRef>{ table, uncompress_unsigned(data) };
            break;

        case ElementType::GenericInst:
            return GenericTypeInstSig{ table, data };
            break;

        case ElementType::Var:
            return GenericTypeIndex{ uncompress_unsigned(data) };
            break;

        case ElementType::MVar:
            return GenericMethodTypeIndex{ uncompress_unsigned(data) };
            break;

        default:
            throw_invalid("Unrecognized ELEMENT_TYPE encountered");
            break;
        }
    }
}
