
WINRT_EXPORT namespace xlang
{
    void check_hresult(hresult result);

    template <typename T>
    struct com_ptr;

    template <typename T, typename = std::enable_if_t<!std::is_base_of_v<Windows::Foundation::IUnknown, T>>>
    auto get_abi(T const& object) noexcept
    {
        return reinterpret_cast<impl::abi_t<T> const&>(object);
    }

    template <typename T, typename = std::enable_if_t<!std::is_base_of_v<Windows::Foundation::IUnknown, T>>>
    auto put_abi(T& object) noexcept
    {
        return reinterpret_cast<impl::abi_t<T>*>(&object);
    }

    template <typename T, typename V, typename = std::enable_if_t<!std::is_base_of_v<Windows::Foundation::IUnknown, T>>>
    void copy_from_abi(T& object, V&& value)
    {
        object = reinterpret_cast<T const&>(value);
    }

    template <typename T, typename V, typename = std::enable_if_t<!std::is_base_of_v<Windows::Foundation::IUnknown, T>>>
    void copy_to_abi(T const& object, V& value)
    {
        reinterpret_cast<T&>(value) = object;
    }

    template <typename T, typename = std::enable_if_t<!std::is_base_of_v<Windows::Foundation::IUnknown, std::decay_t<T>> && !std::is_convertible_v<T, std::wstring_view>>>
    auto detach_abi(T&& object)
    {
        impl::abi_t<T> result{};
        reinterpret_cast<T&>(result) = std::move(object);
        return result;
    }

    struct take_ownership_from_abi_t {};
    constexpr take_ownership_from_abi_t take_ownership_from_abi{};
}

namespace xlang::impl
{
    template <typename T>
    using com_ref = std::conditional_t<std::is_base_of_v<Windows::Foundation::IUnknown, T>, T, com_ptr<T>>;

    template <typename D, typename I, typename Enable = void>
    struct produce_base;

    template <typename D, typename I>
    struct produce : produce_base<D, I>
    {
    };

    template <typename D, typename Enable = void>
    struct get_self_abi
    {
        static void* value(void* result) noexcept
        {
            return result;
        }
    };

    template <typename D>
    struct get_self_abi<D, std::enable_if_t<is_implements_v<D>>>
    {
        static void* value(void* result) noexcept
        {
            return &static_cast<produce<D, typename default_interface<D>::type>*>(result)->shim();
        }
    };

    template <typename To, typename From>
    com_ref<To> as(From* ptr)
    {
        if (!ptr)
        {
            return nullptr;
        }

        void* result;
        check_hresult(ptr->QueryInterface(guid_of<To>(), &result));
        return { take_ownership_from_abi, get_self_abi<To>::value(result) };
    }

    template <typename To, typename From>
    com_ref<To> try_as(From* ptr) noexcept
    {
        if (!ptr)
        {
            return nullptr;
        }

        void* result;
        ptr->QueryInterface(guid_of<To>(), &result);
        return { take_ownership_from_abi, get_self_abi<To>::value(result) };
    }

    template <typename T>
    struct wrapped_type
    {
        using type = T;
    };

    template <typename T>
    struct wrapped_type<com_ptr<T>>
    {
        using type = T;
    };

    template <typename T>
    using wrapped_type_t = typename wrapped_type<T>::type;
}

WINRT_EXPORT namespace xlang::Windows::Foundation
{
    struct IUnknown
    {
        IUnknown() noexcept = default;
        IUnknown(std::nullptr_t) noexcept {}
        void* operator new(size_t) = delete;

        IUnknown(take_ownership_from_abi_t, void* ptr) noexcept : m_ptr(static_cast<impl::unknown_abi*>(ptr))
        {
        }

        IUnknown(IUnknown const& other) noexcept : m_ptr(other.m_ptr)
        {
            add_ref();
        }

        IUnknown(IUnknown&& other) noexcept : m_ptr(std::exchange(other.m_ptr, {}))
        {
        }

        ~IUnknown() noexcept
        {
            release_ref();
        }

        IUnknown& operator=(IUnknown const& other) noexcept
        {
            if (this != &other)
            {
                release_ref();
                m_ptr = other.m_ptr;
                add_ref();
            }

            return*this;
        }

        IUnknown& operator=(IUnknown&& other) noexcept
        {
            if (this != &other)
            {
                release_ref();
                m_ptr = std::exchange(other.m_ptr, {});
            }

            return*this;
        }

        explicit operator bool() const noexcept
        {
            return nullptr != m_ptr;
        }

        IUnknown& operator=(std::nullptr_t) noexcept
        {
            release_ref();
            return*this;
        }

        template <typename To>
        auto as() const
        {
            return impl::as<To>(m_ptr);
        }

        template <typename To>
        auto try_as() const noexcept
        {
            return impl::try_as<To>(m_ptr);
        }

        template <typename To>
        void as(To& to) const
        {
            to = as<impl::wrapped_type_t<To>>();
        }

        template <typename To>
        bool try_as(To& to) const noexcept
        {
            to = try_as<impl::wrapped_type_t<To>>();
            return static_cast<bool>(to);
        }

        hresult as(guid const& id, void** result) const noexcept
        {
            return m_ptr->QueryInterface(id, result);
        }

        friend void swap(IUnknown& left, IUnknown& right) noexcept
        {
            std::swap(left.m_ptr, right.m_ptr);
        }

    private:

        void add_ref() const noexcept
        {
            if (m_ptr)
            {
                m_ptr->AddRef();
            }
        }

        void release_ref() noexcept
        {
            if (m_ptr)
            {
                unconditional_release_ref();
            }
        }

        WINRT_NOINLINE void unconditional_release_ref() noexcept
        {
            std::exchange(m_ptr, {})->Release();
        }

        impl::unknown_abi* m_ptr{};
    };
}

WINRT_EXPORT namespace xlang
{
    inline void* get_abi(Windows::Foundation::IUnknown const& object) noexcept
    {
        return *(void**)(&object);
    }

    inline void** put_abi(Windows::Foundation::IUnknown& object) noexcept
    {
        WINRT_ASSERT(get_abi(object) == nullptr);
        return reinterpret_cast<void**>(&object);
    }

    inline void attach_abi(Windows::Foundation::IUnknown& object, void* value) noexcept
    {
        object = nullptr;
        *put_abi(object) = value;
    }

    inline void* detach_abi(Windows::Foundation::IUnknown& object) noexcept
    {
        void* temp = get_abi(object);
        *reinterpret_cast<void**>(&object) = nullptr;
        return temp;
    }

    inline void* detach_abi(Windows::Foundation::IUnknown&& object) noexcept
    {
        void* temp = get_abi(object);
        *reinterpret_cast<void**>(&object) = nullptr;
        return temp;
    }

    constexpr void* detach_abi(std::nullptr_t) noexcept
    {
        return nullptr;
    }

    inline void copy_from_abi(Windows::Foundation::IUnknown& object, void* value) noexcept
    {
        object = nullptr;

        if (value)
        {
            static_cast<impl::unknown_abi*>(value)->AddRef();
            *put_abi(object) = value;
        }
    }

    inline void copy_to_abi(Windows::Foundation::IUnknown const& object, void*& value) noexcept
    {
        WINRT_ASSERT(value == nullptr);
        value = get_abi(object);

        if (value)
        {
            static_cast<impl::unknown_abi*>(value)->AddRef();
        }
    }

#ifdef WINRT_WINDOWS_ABI

    inline ::IUnknown* get_unknown(Windows::Foundation::IUnknown const& object) noexcept
    {
        return static_cast<::IUnknown*>(get_abi(object));
    }

#endif
}

WINRT_EXPORT namespace xlang::Windows::Foundation
{
    inline bool operator==(IUnknown const& left, IUnknown const& right) noexcept
    {
        if (get_abi(left) == get_abi(right))
        {
            return true;
        }
        if (!left || !right)
        {
            return false;
        }
        return get_abi(left.try_as<IUnknown>()) == get_abi(right.try_as<IUnknown>());
    }

    inline bool operator!=(IUnknown const& left, IUnknown const& right) noexcept
    {
        return !(left == right);
    }

    inline bool operator<(IUnknown const& left, IUnknown const& right) noexcept
    {
        if (get_abi(left) == get_abi(right))
        {
            return false;
        }
        if (!left || !right)
        {
            return get_abi(left) < get_abi(right);
        }
        return get_abi(left.try_as<IUnknown>()) < get_abi(right.try_as<IUnknown>());
    }

    inline bool operator>(IUnknown const& left, IUnknown const& right) noexcept
    {
        return right < left;
    }

    inline bool operator<=(IUnknown const& left, IUnknown const& right) noexcept
    {
        return !(right < left);
    }

    inline bool operator>=(IUnknown const& left, IUnknown const& right) noexcept
    {
        return !(left < right);
    }

    struct IInspectable : IUnknown
    {
        IInspectable(std::nullptr_t = nullptr) noexcept {}
        IInspectable(take_ownership_from_abi_t, void* ptr) noexcept : IUnknown(take_ownership_from_abi, ptr) {}
    };
}
