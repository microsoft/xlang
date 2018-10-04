
WINRT_WARNING_PUSH

WINRT_EXPORT namespace winrt
{
    template <typename Interface = Windows::Foundation::IActivationFactory>
    auto get_activation_factory(param::hstring const& name)
    {
        impl::com_ref<Interface> object;
        hresult hr = WINRT_RoGetActivationFactory(get_abi(name), guid_of<Interface>(), put_abi(object));

        if (hr == impl::error_not_initialized)
        {
            void* cookie{};
            check_hresult(WINRT_CoIncrementMTAUsage(&cookie));
            hr = WINRT_RoGetActivationFactory(get_abi(name), guid_of<Interface>(), put_abi(object));
        }

        check_hresult(hr);
        return object;
    }
}

namespace winrt::impl
{
    inline int32_t interlocked_read_32(int32_t const volatile* target) noexcept
    {
#if defined _M_IX86 || defined _M_X64
        int32_t const result = *target;
        _ReadWriteBarrier();
        return result;
#elif defined _M_ARM || defined _M_ARM64
        int32_t const result = __iso_volatile_load32(reinterpret_cast<int32_t const volatile*>(target));
        WINRT_INTERLOCKED_READ_MEMORY_BARRIER
            return result;
#else
#error Unsupported architecture
#endif
    }

#if defined _WIN64
    inline int64_t interlocked_read_64(int64_t const volatile* target) noexcept
    {
#if defined _M_X64
        int64_t const result = *target;
        _ReadWriteBarrier();
        return result;
#elif defined _M_ARM64
        int64_t const result = __iso_volatile_load64(target);
        WINRT_INTERLOCKED_READ_MEMORY_BARRIER
            return result;
#else
#error Unsupported architecture
#endif
    }
#endif

    template <typename T>
    T* interlocked_read_pointer(T* const volatile* target) noexcept
    {
#ifdef _WIN64
        return (T*)interlocked_read_64((int64_t*)target);
#else
        return (T*)interlocked_read_32((int32_t*)target);
#endif
    }

#ifdef _WIN64
    inline constexpr uint32_t memory_allocation_alignment{ 16 };
#pragma warning(push)
#pragma warning(disable:4324) // structure was padded due to alignment specifier
    struct alignas(16) slist_entry
    {
        slist_entry* next;
    };
    union alignas(16) slist_header
    {
        struct
        {
            uint64_t reserved1;
            uint64_t reserved2;
        } reserved1;
        struct
        {
            uint64_t reserved1 : 16;
            uint64_t reserved2 : 48;
            uint64_t reserved3 : 4;
            uint64_t reserved4 : 60;
        } reserved2;
    };
#pragma warning(pop)
#else
    inline constexpr uint32_t memory_allocation_alignment{ 8 };
    struct slist_entry
    {
        slist_entry* next;
    };
    union slist_header
    {
        uint64_t reserved1;
        struct
        {
            slist_entry reserved1;
            uint16_t reserved2;
            uint16_t reserved3;
        } reserved2;
    };
#endif

    struct factory_cache_typeless_entry
    {
        struct alignas(sizeof(void*) * 2) object_and_count
        {
            IUnknown* pointer;
            size_t count;
        };

        object_and_count value;
        alignas(memory_allocation_alignment) slist_entry next {};

        void clear() noexcept
        {
            IUnknown* pointer_value = interlocked_read_pointer(&value.pointer);

            if (pointer_value == nullptr)
            {
                return;
            }

            object_and_count current_value{ pointer_value, 0 };

#if defined _WIN64
            if (1 == _InterlockedCompareExchange128((int64_t*)this, 0, 0, (int64_t*)&current_value))
            {
                pointer_value->Release();
            }
#else
            int64_t const result = _InterlockedCompareExchange64((int64_t*)this, 0, *(int64_t*)&current_value);

            if (result == *(int64_t*)&current_value)
            {
                pointer_value->Release();
            }
#endif
        }
    };

    struct factory_cache
    {
        factory_cache(factory_cache const&) = delete;
        factory_cache& operator=(factory_cache const&) = delete;

        factory_cache() noexcept
        {
            WINRT_InitializeSListHead(&m_list);
        }

        void add(factory_cache_typeless_entry* const entry) noexcept
        {
            WINRT_ASSERT(entry);
            WINRT_InterlockedPushEntrySList(&m_list, &entry->next);
        }

        void clear() noexcept
        {
            slist_entry* entry = static_cast<slist_entry*>(WINRT_InterlockedFlushSList(&m_list));

            while (entry != nullptr)
            {
                // entry->Next must be read before entry->clear() is called since the InterlockedCompareExchange
                // inside clear() will allow another thread to add the entry back to the cache.
                slist_entry* next = entry->next;
                reinterpret_cast<factory_cache_typeless_entry*>(reinterpret_cast<uint8_t*>(entry) - offsetof(factory_cache_typeless_entry, next))->clear();
                entry = next;
            }
        }

    private:

        alignas(memory_allocation_alignment) slist_header m_list;
    };

    inline factory_cache& get_factory_cache() noexcept
    {
        static factory_cache cache;
        return cache;
    }

    template <typename Class, typename Interface>
    struct factory_cache_entry
    {
        template <typename F>
        auto call(F&& callback)
        {
#ifdef WINRT_DIAGNOSTICS
            get_diagnostics_info().add_factory<Class>();
#endif

            {
                count_guard const guard(m_value.count);

                if (m_value.object)
                {
                    return callback(*reinterpret_cast<com_ref<Interface> const*>(&m_value.object));
                }
            }

            auto object = get_activation_factory<Interface>(name_of<Class>());

            if (!object.template try_as<IAgileObject>())
            {
#ifdef WINRT_DIAGNOSTICS
                get_diagnostics_info().non_agile_factory<Class>();
#endif
                return callback(object);
            }

            {
                count_guard const guard(m_value.count);

                if (nullptr == _InterlockedCompareExchangePointer((void**)&m_value.object, get_abi(object), nullptr))
                {
                    // This thread successfully updated the entry to hold the factory object. We thus detach, since the
                    // factory_cache_entry now owns the reference, and add the entry to the cache list. The callback
                    // may be safely called using the cached object since the count guard is currently being held.
                    detach_abi(object);
                    get_factory_cache().add(reinterpret_cast<factory_cache_typeless_entry*>(this));
                    return callback(*reinterpret_cast<com_ref<Interface> const*>(&m_value.object));
                }
                else
                {
                    // This thread failed to update the entry since another thread managed to exchange pointers first.
                    // The callback must still be called and can simply use the temporary factory object before allowing
                    // it to be released. 
                    return callback(object);
                }
            }
        }

    private:

        struct count_guard
        {
            count_guard(count_guard const&) = delete;
            count_guard& operator=(count_guard const&) = delete;

            explicit count_guard(size_t& count) noexcept : m_count(count)
            {
#ifdef _WIN64
                _InterlockedIncrement64((int64_t*)&m_count);
#else
                _InterlockedIncrement((long*)&m_count);
#endif
            }

            ~count_guard() noexcept
            {
#ifdef _WIN64
                _InterlockedDecrement64((int64_t*)&m_count);
#else
                _InterlockedDecrement((long*)&m_count);
#endif
            }

        private:

            size_t& m_count;
        };

        struct alignas(sizeof(void*) * 2) object_and_count
        {
            void* object;
            size_t count;
        };

        object_and_count m_value;
        alignas(memory_allocation_alignment) slist_entry m_next;
    };

    template <typename Class, typename Interface>
    struct factory_storage
    {
        static factory_cache_entry<Class, Interface> factory;
    };

    template <typename Class, typename Interface>
    factory_cache_entry<Class, Interface> factory_storage<Class, Interface>::factory;

    template <typename Class, typename Interface = Windows::Foundation::IActivationFactory, typename F>
    auto call_factory(F&& callback)
    {
        static_assert(sizeof(factory_cache_typeless_entry) == sizeof(factory_cache_entry<Class, Interface>));
        static_assert(std::alignment_of_v<factory_cache_typeless_entry> == std::alignment_of_v<factory_cache_entry<Class, Interface>>);
        static_assert(std::is_standard_layout_v<factory_cache_typeless_entry>);
        static_assert(std::is_standard_layout_v<factory_cache_entry<Class, Interface>>);

        return factory_storage<Class, Interface>::factory.call(callback);
    }

    template <typename Class, typename Interface = Windows::Foundation::IActivationFactory>
    auto try_get_activation_factory(hresult_error* exception = nullptr) noexcept
    {
        param::hstring const name{ name_of<Class>() };
        impl::com_ref<Interface> object;
        hresult const hr = WINRT_RoGetActivationFactory(get_abi(name), guid_of<Interface>(), put_abi(object));

        if (hr < 0)
        {
            // Ensure that the IRestrictedErrorInfo is not left on the thread.
            hresult_error local_exception{ hr, hresult_error::from_abi };

            if (exception)
            {
                // Optionally transfer ownership to the caller.
                *exception = std::move(local_exception);
            }
        }

        return object;
    }
}

WINRT_EXPORT namespace winrt
{
    namespace Windows::Foundation
    {
        struct IActivationFactory :
            IInspectable,
            impl::consume_t<IActivationFactory>
        {
            IActivationFactory(std::nullptr_t = nullptr) noexcept {}
        };
    }

    enum class apartment_type : int32_t
    {
        single_threaded,
        multi_threaded
    };

    inline void init_apartment(apartment_type const type = apartment_type::multi_threaded)
    {
        hresult const result = WINRT_RoInitialize(static_cast<uint32_t>(type));

        if (result < 0)
        {
            throw_hresult(result);
        }
    }

    inline void uninit_apartment() noexcept
    {
        WINRT_RoUninitialize();
    }

    template <typename Class, typename Interface = Windows::Foundation::IActivationFactory>
    auto get_activation_factory()
    {
        // Normally, the callback avoids having to return a ref-counted object and the resulting AddRef/Release bump.
        // In this case we do want a unique reference, so we use the lambda to return one and thus produce an
        // AddRef'd object that is returned to the caller. 
        return impl::call_factory<Class, Interface>([](auto&& factory)
        {
            return factory;
        });
    }

    template <typename Class, typename Interface = Windows::Foundation::IActivationFactory>
    auto try_get_activation_factory() noexcept
    {
        return impl::try_get_activation_factory<Class, Interface>();
    }

    template <typename Class, typename Interface = Windows::Foundation::IActivationFactory>
    auto try_get_activation_factory(hresult_error& exception) noexcept
    {
        return impl::try_get_activation_factory<Class, Interface>(&exception);
    }

    inline void clear_factory_cache() noexcept
    {
        impl::get_factory_cache().clear();
    }

    template <typename Interface>
    impl::com_ref<Interface> create_instance(guid const& clsid, uint32_t context = 0x1 /*CLSCTX_INPROC_SERVER*/, void* outer = nullptr)
    {
        impl::com_ref<Interface> temp{ nullptr };
        check_hresult(WINRT_CoCreateInstance(clsid, outer, context, guid_of<Interface>(), put_abi(temp)));
        return temp;
    }
}

WINRT_WARNING_POP
