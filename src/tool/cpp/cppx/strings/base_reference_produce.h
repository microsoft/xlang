
namespace xlang::impl
{
    template <typename D, typename T>
    struct produce<D, Runtime::IReference<T>> : produce_base<D, Runtime::IReference<T>>
    {
        int32_t WINRT_CALL get_Value(arg_out<T> value) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                *value = detach_from<T>(this->shim().Value());
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }
    };

    template <typename D, typename T>
    struct produce<D, Runtime::IReferenceArray<T>> : produce_base<D, Runtime::IReferenceArray<T>>
    {
        int32_t WINRT_CALL get_Value(uint32_t* __valueSize, arg_out<T>* value) noexcept final
        {
            try
            {
                *__valueSize = 0;
                *value = nullptr;
                typename D::abi_guard guard(this->shim());
                std::tie(*__valueSize, *value) = detach_abi(this->shim().Value());
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }
    };

    template <typename T>
    struct reference final : implements<reference<T>, Runtime::IReference<T>, Runtime::IPropertyValue>
    {
        reference(T const& value) : m_value(value)
        {
        }

        T Value() const
        {
            return m_value;
        }

        Runtime::PropertyType Type() const noexcept
        {
            return Runtime::PropertyType::OtherType;
        }

        static constexpr bool IsNumericScalar() noexcept
        {
            return std::is_arithmetic_v<T> || std::is_enum_v<T>;
        }

        uint8_t GetUInt8() const
        {
            return to_scalar<uint8_t>();
        }

        int16_t GetInt16() const
        {
            return to_scalar<int16_t>();
        }

        uint16_t GetUInt16() const
        {
            return to_scalar<uint16_t>();
        }

        int32_t GetInt32() const
        {
            return to_scalar<int32_t>();
        }

        uint32_t GetUInt32() const
        {
            return to_scalar<uint32_t>();
        }

        int64_t GetInt64() const
        {
            return to_scalar<int64_t>();
        }

        uint64_t GetUInt64() const
        {
            return to_scalar<uint64_t>();
        }

        float GetSingle() { throw hresult_not_implemented(); }
        double GetDouble() { throw hresult_not_implemented(); }
        char16_t GetChar16() { throw hresult_not_implemented(); }
        bool GetBoolean() { throw hresult_not_implemented(); }
        hstring GetString() { throw hresult_not_implemented(); }
        guid GetGuid() { throw hresult_not_implemented(); }
        Runtime::DateTime GetDateTime() { throw hresult_not_implemented(); }
        Runtime::TimeSpan GetTimeSpan() { throw hresult_not_implemented(); }
        Runtime::Point GetPoint() { throw hresult_not_implemented(); }
        Runtime::Size GetSize() { throw hresult_not_implemented(); }
        Runtime::Rect GetRect() { throw hresult_not_implemented(); }
        void GetUInt8Array(com_array<uint8_t> &) { throw hresult_not_implemented(); }
        void GetInt16Array(com_array<int16_t> &) { throw hresult_not_implemented(); }
        void GetUInt16Array(com_array<uint16_t> &) { throw hresult_not_implemented(); }
        void GetInt32Array(com_array<int32_t> &) { throw hresult_not_implemented(); }
        void GetUInt32Array(com_array<uint32_t> &) { throw hresult_not_implemented(); }
        void GetInt64Array(com_array<int64_t> &) { throw hresult_not_implemented(); }
        void GetUInt64Array(com_array<uint64_t> &) { throw hresult_not_implemented(); }
        void GetSingleArray(com_array<float> &) { throw hresult_not_implemented(); }
        void GetDoubleArray(com_array<double> &) { throw hresult_not_implemented(); }
        void GetChar16Array(com_array<char16_t> &) { throw hresult_not_implemented(); }
        void GetBooleanArray(com_array<bool> &) { throw hresult_not_implemented(); }
        void GetStringArray(com_array<hstring> &) { throw hresult_not_implemented(); }
        void GetInspectableArray(com_array<Runtime::IObject> &) { throw hresult_not_implemented(); }
        void GetGuidArray(com_array<guid> &) { throw hresult_not_implemented(); }
        void GetDateTimeArray(com_array<Runtime::DateTime> &) { throw hresult_not_implemented(); }
        void GetTimeSpanArray(com_array<Runtime::TimeSpan> &) { throw hresult_not_implemented(); }
        void GetPointArray(com_array<Runtime::Point> &) { throw hresult_not_implemented(); }
        void GetSizeArray(com_array<Runtime::Size> &) { throw hresult_not_implemented(); }
        void GetRectArray(com_array<Runtime::Rect> &) { throw hresult_not_implemented(); }

    private:

        template <typename To>
        To to_scalar() const
        {
            if constexpr (IsNumericScalar())
            {
                return static_cast<To>(m_value);
            }
            else
            {
                throw hresult_not_implemented();
            }
        }

        T m_value;
    };

    template <typename T>
    struct reference_traits
    {
        static auto make(T const& value) { return xlang::make<impl::reference<T>>(value); }
    };

    template <>
    struct reference_traits<uint8_t>
    {
        static auto make(uint8_t value) { return Runtime::PropertyValue::CreateUInt8(value); }
    };

    template <>
    struct reference_traits<uint16_t>
    {
        static auto make(uint16_t value) { return Runtime::PropertyValue::CreateUInt16(value); }
    };

    template <>
    struct reference_traits<int16_t>
    {
        static auto make(int16_t value) { return Runtime::PropertyValue::CreateInt16(value); }
    };

    template <>
    struct reference_traits<uint32_t>
    {
        static auto make(uint32_t value) { return Runtime::PropertyValue::CreateUInt32(value); }
    };

    template <>
    struct reference_traits<int32_t>
    {
        static auto make(int32_t value) { return Runtime::PropertyValue::CreateInt32(value); }
    };

    template <>
    struct reference_traits<uint64_t>
    {
        static auto make(uint64_t value) { return Runtime::PropertyValue::CreateUInt64(value); }
    };

    template <>
    struct reference_traits<int64_t>
    {
        static auto make(int64_t value) { return Runtime::PropertyValue::CreateInt64(value); }
    };

    template <>
    struct reference_traits<float>
    {
        static auto make(float value) { return Runtime::PropertyValue::CreateSingle(value); }
    };

    template <>
    struct reference_traits<double>
    {
        static auto make(double value) { return Runtime::PropertyValue::CreateDouble(value); }
    };

    template <>
    struct reference_traits<char16_t>
    {
        static auto make(char16_t value) { return Runtime::PropertyValue::CreateChar16(value); }
    };

    template <>
    struct reference_traits<bool>
    {
        static auto make(bool value) { return Runtime::PropertyValue::CreateBoolean(value); }
    };

    template <>
    struct reference_traits<hstring>
    {
        static auto make(hstring const& value) { return Runtime::PropertyValue::CreateString(value); }
    };

    template <>
    struct reference_traits<Runtime::IObject>
    {
        static auto make(Runtime::IObject const& value) { return Runtime::PropertyValue::CreateInspectable(value); }
    };

    template <>
    struct reference_traits<guid>
    {
        static auto make(guid const& value) { return Runtime::PropertyValue::CreateGuid(value); }
    };

    template <>
    struct reference_traits<Runtime::DateTime>
    {
        static auto make(Runtime::DateTime value) { return Runtime::PropertyValue::CreateDateTime(value); }
    };

    template <>
    struct reference_traits<Runtime::TimeSpan>
    {
        static auto make(Runtime::TimeSpan value) { return Runtime::PropertyValue::CreateTimeSpan(value); }
    };

    template <>
    struct reference_traits<Runtime::Point>
    {
        static auto make(Runtime::Point const& value) { return Runtime::PropertyValue::CreatePoint(value); }
    };

    template <>
    struct reference_traits<Runtime::Size>
    {
        static auto make(Runtime::Size const& value) { return Runtime::PropertyValue::CreateSize(value); }
    };

    template <>
    struct reference_traits<Runtime::Rect>
    {
        static auto make(Runtime::Rect const& value) { return Runtime::PropertyValue::CreateRect(value); }
    };
}

WINRT_EXPORT namespace xlang::Runtime
{
    template <typename T>
    struct IReference :
        IObject,
        impl::consume_t<IReference<T>>,
        impl::require<IReference<T>, IPropertyValue>
    {
        static_assert(impl::has_category_v<T>, "T must be WinRT type.");
        IReference(std::nullptr_t = nullptr) noexcept {}
        IReference(take_ownership_from_abi_t, void* ptr) noexcept : IObject(take_ownership_from_abi, ptr) {}

        IReference(T const& value) : IReference<T>(impl::reference_traits<T>::make(value))
        {
        }

    private:

        IReference<T>(IObject const& value) : IReference<T>(value.as<IReference<T>>())
        {
        }
    };

    template <typename T>
    bool operator==(IReference<T> const& left, IReference<T> const& right)
    {
        if (get_abi(left) == get_abi(right))
        {
            return true;
        }

        if (!left || !right)
        {
            return false;
        }

        return left.Value() == right.Value();
    }

    template <typename T>
    bool operator!=(IReference<T> const& left, IReference<T> const& right)
    {
        return !(left == right);
    }

    template <typename T>
    struct WINRT_EBO IReferenceArray :
        IObject,
        impl::consume_t<IReferenceArray<T>>,
        impl::require<IReferenceArray<T>, IPropertyValue>
    {
        static_assert(impl::has_category_v<T>, "T must be WinRT type.");
        IReferenceArray<T>(std::nullptr_t = nullptr) noexcept {}
        IReferenceArray(take_ownership_from_abi_t, void* ptr) noexcept : IObject(take_ownership_from_abi, ptr) {}
    };
}

WINRT_EXPORT namespace xlang
{
    inline Runtime::IObject box_value(param::hstring const& value)
    {
        return Runtime::IReference<hstring>(*(hstring*)(&value));
    }

    template <typename T, typename = std::enable_if_t<!std::is_convertible_v<T, param::hstring>>>
    Runtime::IObject box_value(T const& value)
    {
        if constexpr (std::is_base_of_v<Runtime::IObject, T>)
        {
            return value;
        }
        else
        {
            return Runtime::IReference<T>(value);
        }
    }

    template <typename T>
    T unbox_value(Runtime::IObject const& value)
    {
        if constexpr (std::is_base_of_v<Runtime::IObject, T>)
        {
            return value.as<T>();
        }
        else if constexpr (std::is_enum_v<T>)
        {
            if (auto temp = value.try_as<Runtime::IReference<T>>())
            {
                return temp.Value();
            }
            else
            {
                return static_cast<T>(value.as<Runtime::IReference<std::underlying_type_t<T>>>().Value());
            }
        }
        else
        {
            return value.as<Runtime::IReference<T>>().Value();
        }
    }

    template <typename T>
    hstring unbox_value_or(Runtime::IObject const& value, param::hstring const& default_value)
    {
        if (value)
        {
            if (auto temp = value.try_as<Runtime::IReference<hstring>>())
            {
                return temp.Value();
            }
        }

        return *(hstring*)(&default_value);
    }

    template <typename T, typename = std::enable_if_t<!std::is_same_v<T, hstring>>>
    T unbox_value_or(Runtime::IObject const& value, T const& default_value)
    {
        if (value)
        {
            if constexpr (std::is_base_of_v<Runtime::IObject, T>)
            {
                if (auto temp = value.try_as<T>())
                {
                    return temp;
                }
            }
            else if constexpr (std::is_enum_v<T>)
            {
                if (auto temp = value.try_as<Runtime::IReference<T>>())
                {
                    return temp.Value();
                }

                if (auto temp = value.try_as<Runtime::IReference<std::underlying_type_t<T>>>())
                {
                    return static_cast<T>(temp.Value());
                }
            }
            else
            {
                if (auto temp = value.try_as<Runtime::IReference<T>>())
                {
                    return temp.Value();
                }
            }
        }

        return default_value;
    }
}