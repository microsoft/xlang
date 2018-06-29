
namespace xlang::meta::reader
{
    inline uint32_t uncompress_unsigned(byte_view& cursor, uint32_t& length)
    {
        auto data = cursor.begin();
        uint32_t value;
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
            throw_invalid(u"Invalid compressed integer in blob");
        }
        cursor = cursor.seek(length);
        return value;
    }

    inline uint32_t uncompress_unsigned(byte_view& cursor)
    {
        uint32_t length_ignored;
        return uncompress_unsigned(cursor, length_ignored);
    }

    inline int32_t uncompress_signed(byte_view& cursor, uint32_t length)
    {
        static constexpr uint32_t sign_mask_one_byte{ 0xffffffc0 };
        static constexpr uint32_t sign_mask_two_byte{ 0xffffe000 };
        static constexpr uint32_t sign_mask_four_byte{ 0xf0000000 };

        auto unsigned_data = uncompress_signed(cursor, length);
        const bool negative = (unsigned_data & 0x01) != 0x00;
        unsigned_data >>= 1;
        if (negative)
        {
            switch (length)
            {
            case 1:
                unsigned_data |= sign_mask_one_byte;
                break;

            case 2:
                unsigned_data |= sign_mask_two_byte;
                break;

            default:
                WINRT_ASSERT(length == 4);
                unsigned_data |= sign_mask_four_byte;
                break;
            }
        }
        return static_cast<int32_t>(unsigned_data);
    }

    inline int32_t uncompress_signed(byte_view& cursor)
    {
        uint32_t length_ignored;
        return uncompress_signed(cursor, length_ignored);
    }

    template <typename T>
    T uncompress_enum(byte_view& cursor)
    {
        static_assert(std::is_enum_v<T>);
        if constexpr (std::is_signed_v<std::underlying_type_t<T>>)
        {
            return static_cast<T>(uncompress_signed(cursor));
        }
        else
        {
            return static_cast<T>(uncompress_unsigned(cursor));
        }
    }

    enum class ElementType : uint8_t
    {
        End = 0x00, // Sentinel value

        Void = 0x01,
        Boolean = 0x02,
        Char = 0x03,
        I1 = 0x04,
        U1 = 0x05,
        I2 = 0x06,
        U2 = 0x07,
        I4 = 0x08,
        U4 = 0x09,
        I8 = 0x0a,
        U8 = 0x0b,
        R4 = 0x0c,
        R8 = 0x0d,
        String = 0x0e,

        Ptr         = 0x0f, // Followed by TypeSig
        ByRef       = 0x10, // Followed by TypeSig
        ValueType   = 0x11, // Followed by TypeDef or TypeRef
        Class       = 0x12, // Followed by TypeDef or TypeRef
        Var         = 0x13, // Generic parameter in a type definition, represented as unsigned integer
        Array       = 0x14,
        GenericInst = 0x15,
        TypedByRef  = 0x16,

        I = 0x18, // System.IntPtr
        U = 0x19, // System.UIntPtr

        FnPtr    = 0x1b, // Followed by full method signature
        Object   = 0x1c, // System.Object
        SZArray  = 0x1d,
        MVar     = 0x1e, // Generic parameter in a method definition, represented as unsigned integer
        CModReqd = 0x1f, // Required modifier, followed by a TypeDef or TypeRef
        CModOpt  = 0x20, // Optional modifier, followed by a TypeDef or TypeRef
        Internal = 0x21,

        Modifier = 0x40, // Or'd with folowing element types
        Sentinel = 0x41, // Sentinel for vararg method signature

        Pinned = 0x45,

        Type = 0x50, // System.Type
        TaggedObject = 0x51, // Boxed object (in custom attributes)
        Field = 0x53, // Custom attribute field
        Property = 0x54, // Custom attribute property
        Enum = 0x55, // Custom attribute enum
    };

    enum class CallingConvention : uint8_t
    {
        Default = 0x00,
        VarArg = 0x05,
        Field = 0x06,
        LocalSig = 0x07,
        Property = 0x08,
        GenericInst = 0x10,
        Mask = 0x0f,

        HasThis = 0x20,
        ExplicitThis = 0x40,
        Generic = 0x10,
    };

    inline CallingConvention operator&(CallingConvention lhs, CallingConvention rhs) noexcept
    {
        return static_cast<CallingConvention>(static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs));
    }

    struct CustomModSig;
    struct GenericTypeInstSig;
    struct MethodDefSig;
    struct ParamSig;
    struct RetTypeSig;
    struct TypeSig;
    struct TypeSpecSig;

    struct CustomModSig
    {
        CustomModSig(table_base const* table, byte_view& data)
            : m_cmod(uncompress_enum<ElementType>(data))
            , m_type(table, uncompress_unsigned(data))
        {
            WINRT_ASSERT(m_cmod == ElementType::CModReqd || m_cmod == ElementType::CModOpt);
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
            return m_generic_arg_count;
        }

        auto GenericArgs() const noexcept
        {
            return range{ std::pair{ m_generic_args.cbegin(), m_generic_args.cend() } };
        }

    private:
        ElementType m_class_or_value;
        coded_index<TypeDefOrRef> m_type;
        uint32_t m_generic_arg_count;
        std::vector<TypeSig> m_generic_args;
    };

    struct TypeSig
    {
        using value_type = std::variant<ElementType, coded_index<TypeDefOrRef>, uint32_t, GenericTypeInstSig>;
        TypeSig(table_base const* table, byte_view& data)
            : m_type(ParseType(table, data))
        {}

        value_type const& Type() const noexcept
        {
            return m_type;
        }

    private:
        static value_type ParseType(table_base const* table, byte_view& data);
        value_type m_type;
    };

    std::vector<CustomModSig> parse_cmods(table_base const* table, byte_view& data)
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

    bool is_by_ref(byte_view& data)
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
            WINRT_ASSERT(element_type != ElementType::TypedByRef);
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

        auto CustomMod() const noexcept
        {
            return range{ std::pair{ m_cmod.cbegin(), m_cmod.cend() } };
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

        auto CustomMod() const noexcept
        {
            return range{ std::pair{ m_cmod.cbegin(), m_cmod.cend() } };
        }

        bool ByRef() const noexcept
        {
            return m_byref;
        }

        bool IsVoid() const noexcept
        {
            return !m_type.has_value();
        }

        TypeSig const& Type() const noexcept
        {
            return *m_type;
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
            , m_generic_param_count((m_calling_convention & CallingConvention::Generic) == CallingConvention::Generic ? uncompress_unsigned(data) : 0)
            , m_param_count(uncompress_unsigned(data))
            , m_ret_type(table, data)
        {
            m_params.reserve(m_param_count);
            for (uint32_t count = 0; count < m_param_count; ++count)
            {
                m_params.emplace_back(table, data);
            }
        }

        CallingConvention CallConvention() const noexcept
        {
            return m_calling_convention;
        }

        uint32_t GenericParamCount() const noexcept
        {
            return m_generic_param_count;
        }

        uint32_t ParamCount() const noexcept
        {
            return m_param_count;
        }

        RetTypeSig const& ReturnType() const noexcept
        {
            return m_ret_type;
        }

        auto Params() const noexcept
        {
            return range{ std::pair{ m_params.cbegin(), m_params.cend() } };
        }

    private:
        CallingConvention m_calling_convention;
        uint32_t m_generic_param_count;
        uint32_t m_param_count;
        RetTypeSig m_ret_type;
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
            auto element_type = uncompress_enum<ElementType>(data);
            WINRT_ASSERT(element_type == ElementType::GenericInst);
            return { table, data };
        }
        GenericTypeInstSig m_type;
    };

    GenericTypeInstSig::GenericTypeInstSig(table_base const* table, byte_view& data)
        : m_class_or_value(uncompress_enum<ElementType>(data))
        , m_type(table, uncompress_unsigned(data))
        , m_generic_arg_count(uncompress_unsigned(data))
    {
        if (!(m_class_or_value == ElementType::Class || m_class_or_value == ElementType::ValueType))
        {
            throw_invalid(u"Generic type instantiation signatures must begin with either ELEMENT_TYPE_CLASS or ELEMENT_TYPE_VALUE");
        }

        m_generic_args.reserve(m_generic_arg_count);
        for (uint32_t arg = 0; arg < m_generic_arg_count; ++arg)
        {
            m_generic_args.emplace_back(table, data);
        }
    }

    TypeSig::value_type TypeSig::ParseType(table_base const* table, byte_view& data)
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
            return uncompress_unsigned(data);
            break;

        default:
            throw_invalid(u"Unrecognized ELEMENT_TYPE encountered");
            break;
        }
    }
}
