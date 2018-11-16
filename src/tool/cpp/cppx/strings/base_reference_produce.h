
namespace xlang::impl
{
    template <typename D, typename T>
    struct produce<D, System::IReference<T>> : produce_base<D, System::IReference<T>>
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
    struct produce<D, System::IReferenceArray<T>> : produce_base<D, System::IReferenceArray<T>>
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
    struct reference final : implements<reference<T>, System::IReference<T>, System::IPropertyValue>
    {
        reference(T const& value) : m_value(value)
        {
        }

        T Value() const
        {
            return m_value;
        }

        System::PropertyType Type() const noexcept
        {
            return System::PropertyType::OtherType;
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
        System::DateTime GetDateTime() { throw hresult_not_implemented(); }
        System::TimeSpan GetTimeSpan() { throw hresult_not_implemented(); }
        System::Point GetPoint() { throw hresult_not_implemented(); }
        System::Size GetSize() { throw hresult_not_implemented(); }
        System::Rect GetRect() { throw hresult_not_implemented(); }
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
        void GetInspectableArray(com_array<System::IObject> &) { throw hresult_not_implemented(); }
        void GetGuidArray(com_array<guid> &) { throw hresult_not_implemented(); }
        void GetDateTimeArray(com_array<System::DateTime> &) { throw hresult_not_implemented(); }
        void GetTimeSpanArray(com_array<System::TimeSpan> &) { throw hresult_not_implemented(); }
        void GetPointArray(com_array<System::Point> &) { throw hresult_not_implemented(); }
        void GetSizeArray(com_array<System::Size> &) { throw hresult_not_implemented(); }
        void GetRectArray(com_array<System::Rect> &) { throw hresult_not_implemented(); }

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
        static auto make(uint8_t value) { return System::PropertyValue::CreateUInt8(value); }
    };

    template <>
    struct reference_traits<uint16_t>
    {
        static auto make(uint16_t value) { return System::PropertyValue::CreateUInt16(value); }
    };

    template <>
    struct reference_traits<int16_t>
    {
        static auto make(int16_t value) { return System::PropertyValue::CreateInt16(value); }
    };

    template <>
    struct reference_traits<uint32_t>
    {
        static auto make(uint32_t value) { return System::PropertyValue::CreateUInt32(value); }
    };

    template <>
    struct reference_traits<int32_t>
    {
        static auto make(int32_t value) { return System::PropertyValue::CreateInt32(value); }
    };

    template <>
    struct reference_traits<uint64_t>
    {
        static auto make(uint64_t value) { return System::PropertyValue::CreateUInt64(value); }
    };

    template <>
    struct reference_traits<int64_t>
    {
        static auto make(int64_t value) { return System::PropertyValue::CreateInt64(value); }
    };

    template <>
    struct reference_traits<float>
    {
        static auto make(float value) { return System::PropertyValue::CreateSingle(value); }
    };

    template <>
    struct reference_traits<double>
    {
        static auto make(double value) { return System::PropertyValue::CreateDouble(value); }
    };

    template <>
    struct reference_traits<char16_t>
    {
        static auto make(char16_t value) { return System::PropertyValue::CreateChar16(value); }
    };

    template <>
    struct reference_traits<bool>
    {
        static auto make(bool value) { return System::PropertyValue::CreateBoolean(value); }
    };

    template <>
    struct reference_traits<hstring>
    {
        static auto make(hstring const& value) { return System::PropertyValue::CreateString(value); }
    };

    template <>
    struct reference_traits<System::IObject>
    {
        static auto make(System::IObject const& value) { return System::PropertyValue::CreateInspectable(value); }
    };

    template <>
    struct reference_traits<guid>
    {
        static auto make(guid const& value) { return System::PropertyValue::CreateGuid(value); }
    };

    template <>
    struct reference_traits<System::DateTime>
    {
        static auto make(System::DateTime value) { return System::PropertyValue::CreateDateTime(value); }
    };

    template <>
    struct reference_traits<System::TimeSpan>
    {
        static auto make(System::TimeSpan value) { return System::PropertyValue::CreateTimeSpan(value); }
    };

    template <>
    struct reference_traits<System::Point>
    {
        static auto make(System::Point const& value) { return System::PropertyValue::CreatePoint(value); }
    };

    template <>
    struct reference_traits<System::Size>
    {
        static auto make(System::Size const& value) { return System::PropertyValue::CreateSize(value); }
    };

    template <>
    struct reference_traits<System::Rect>
    {
        static auto make(System::Rect const& value) { return System::PropertyValue::CreateRect(value); }
    };
}

WINRT_EXPORT namespace xlang::System
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
    inline System::IObject box_value(param::hstring const& value)
    {
        return System::IReference<hstring>(*(hstring*)(&value));
    }

    template <typename T, typename = std::enable_if_t<!std::is_convertible_v<T, param::hstring>>>
    System::IObject box_value(T const& value)
    {
        if constexpr (std::is_base_of_v<System::IObject, T>)
        {
            return value;
        }
        else
        {
            return System::IReference<T>(value);
        }
    }

    template <typename T>
    T unbox_value(System::IObject const& value)
    {
        if constexpr (std::is_base_of_v<System::IObject, T>)
        {
            return value.as<T>();
        }
        else if constexpr (std::is_enum_v<T>)
        {
            if (auto temp = value.try_as<System::IReference<T>>())
            {
                return temp.Value();
            }
            else
            {
                return static_cast<T>(value.as<System::IReference<std::underlying_type_t<T>>>().Value());
            }
        }
        else
        {
            return value.as<System::IReference<T>>().Value();
        }
    }

    template <typename T>
    hstring unbox_value_or(System::IObject const& value, param::hstring const& default_value)
    {
        if (value)
        {
            if (auto temp = value.try_as<System::IReference<hstring>>())
            {
                return temp.Value();
            }
        }

        return *(hstring*)(&default_value);
    }

    template <typename T, typename = std::enable_if_t<!std::is_same_v<T, hstring>>>
    T unbox_value_or(System::IObject const& value, T const& default_value)
    {
        if (value)
        {
            if constexpr (std::is_base_of_v<System::IObject, T>)
            {
                if (auto temp = value.try_as<T>())
                {
                    return temp;
                }
            }
            else if constexpr (std::is_enum_v<T>)
            {
                if (auto temp = value.try_as<System::IReference<T>>())
                {
                    return temp.Value();
                }

                if (auto temp = value.try_as<System::IReference<std::underlying_type_t<T>>>())
                {
                    return static_cast<T>(temp.Value());
                }
            }
            else
            {
                if (auto temp = value.try_as<System::IReference<T>>())
                {
                    return temp.Value();
                }
            }
        }

        return default_value;
    }
}