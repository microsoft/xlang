
WINRT_EXPORT namespace winrt
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

    // TODO: take_ownership_from_abi ?
    struct construct_from_abi_t {};
    inline constexpr construct_from_abi_t construct_from_abi{};
}

namespace winrt::impl
{
    template <typename T>
    using com_ref = std::conditional_t<std::is_base_of_v<Windows::Foundation::IUnknown, T>, T, com_ptr<T>>;

    template <typename D, typename I, typename Enable = void>
    struct produce_base;

    template <typename D, typename I>
    struct produce : produce_base<D, I>
    {
    };

    template <typename T, typename = std::void_t<>>
    struct has_fast_class_type : std::false_type {};

    template <typename T>
    struct has_fast_class_type<T, std::void_t<typename T::fast_class_type>> : std::true_type {};

    template <typename D, typename I, typename Enable = void>
    struct get_self_type
    {
        using type = produce<D, typename default_interface<I>::type>;
    };

    template <typename D, typename I>
    struct get_self_type<D, I, std::enable_if_t<has_fast_class_type<D>::value>>
    {
        using type = produce<D, Windows::Foundation::IInspectable>;
    };

    template <typename To>
    com_ref<To> as(unknown_abi* ptr)
    {
        if (!ptr)
        {
            return nullptr;
        }

        if constexpr (is_implements_v<To>)
        {
            void* result;
            check_hresult(ptr->QueryInterface(guid_of<To>(), &result));
            return { construct_from_abi, &static_cast<typename get_self_type<To, winrt::default_interface<To>>::type*>(result)->shim() };
        }
        else
        {
            void* result;
            check_hresult(ptr->QueryInterface(guid_of<To>(), &result));
            return { construct_from_abi, result };
        }
    }

    template <typename To>
    com_ref<To> try_as(unknown_abi* ptr) noexcept
    {
        if (!ptr)
        {
            return nullptr;
        }

        if constexpr (is_implements_v<To>)
        {
            void* result;
            ptr->QueryInterface(guid_of<To>(), &result);
            return { construct_from_abi, &static_cast<typename get_self_type<To, winrt::default_interface<To>>::type*>(result)->shim() };
        }
        else
        {
            void* result;
            ptr->QueryInterface(guid_of<To>(), &result);
            return { construct_from_abi, result };
        }
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

WINRT_EXPORT namespace winrt::Windows::Foundation
{
    struct IUnknown
    {
        IUnknown() noexcept = default;
        IUnknown(std::nullptr_t) noexcept {}
        void* operator new(size_t) = delete;

        IUnknown(construct_from_abi_t, void* ptr) noexcept : m_ptr(static_cast<impl::unknown_abi*>(ptr))
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

        void attach_abi(void* ptr) noexcept
        {
            release_ref();
            m_ptr = static_cast<impl::unknown_abi*>(ptr);
        }

        void* detach_abi() noexcept
        {
            return std::exchange(m_ptr, nullptr);
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

namespace winrt::impl
{
    template <typename T>
    struct put_abi_type
    {
        put_abi_type(T& object) noexcept : m_object(object)
        {
        }

        operator void**() noexcept
        {
            return &m_ptr;
        }

        void*& operator *() noexcept
        {
            return m_ptr;
        }

        ~put_abi_type() noexcept
        {
            m_object.attach_abi(m_ptr);
        }

    private:

        T& m_object;
        void* m_ptr;
    };
}

WINRT_EXPORT namespace winrt
{
    inline void* get_abi(Windows::Foundation::IUnknown const& object) noexcept
    {
        return *(void**)(&object);
    }

    template <typename T, std::enable_if_t<std::is_base_of_v<Windows::Foundation::IUnknown, T>>* = nullptr>
    auto put_abi(T& object) noexcept
    {
        return impl::put_abi_type<T>{ object };
    }

    template <typename T>
    auto attach_abi(T& object, void* value) noexcept -> std::enable_if_t<std::is_base_of_v<Windows::Foundation::IUnknown, T>>
    {
        object.attach_abi(value);
    }

    template <typename T>
    auto detach_abi(T& object) noexcept -> std::enable_if_t<std::is_base_of_v<Windows::Foundation::IUnknown, T>, void*>
    {
        return object.detach_abi();
    }

    template <typename T>
    auto detach_abi(T&& object) noexcept -> std::enable_if_t<std::is_base_of_v<Windows::Foundation::IUnknown, T>, void*>
    {
        return object.detach_abi();
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

WINRT_EXPORT namespace winrt::Windows::Foundation
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
        IInspectable(construct_from_abi_t, void* ptr) noexcept : IUnknown(construct_from_abi, ptr) {}
    };
}
