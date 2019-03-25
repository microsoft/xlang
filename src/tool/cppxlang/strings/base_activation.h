
namespace winrt
{
    template <typename Interface = Windows::Foundation::IActivationFactory>
    impl::com_ref<Interface> get_activation_factory(param::hstring const& name)
    {
        void* result;
        hresult hr = WINRT_RoGetActivationFactory(get_abi(name), guid_of<Interface>(), &result);

        if (hr == impl::error_not_initialized)
        {
            void* cookie;
            WINRT_CoIncrementMTAUsage(&cookie);
            hr = WINRT_RoGetActivationFactory(get_abi(name), guid_of<Interface>(), &result);
        }

        check_hresult(hr);
        return { result, take_ownership_from_abi };
    }
}

namespace winrt::impl
{
    template <typename Class, typename Interface>
    struct factory_cache_entry
    {
        template <typename F>
        auto call(F&& callback)
        {
            void* value = m_value.load(std::memory_order_acquire);
            if (value)
            {
                return callback(*reinterpret_cast<com_ref<Interface> const*>(&value));
            }

            auto object = get_activation_factory<Interface>(name_of<Class>());
            if (m_value.compare_exchange_strong(value, get_abi(object), std::memory_order_acq_rel))
            {
                value = detach_abi(object);
            }
            return callback(*reinterpret_cast<com_ref<Interface> const*>(&value));
        }

    private:
        std::atomic<void*> m_value{};
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
        static_assert(std::is_standard_layout_v<factory_cache_entry<Class, Interface>>);

        return factory_storage<Class, Interface>::factory.call(callback);
    }

    template <typename Class, typename Interface = Windows::Foundation::IActivationFactory>
    impl::com_ref<Interface> try_get_activation_factory(hresult_error* exception = nullptr) noexcept
    {
        param::hstring const name{ name_of<Class>() };
        void* result;
        hresult const hr = WINRT_RoGetActivationFactory(get_abi(name), guid_of<Interface>(), &result);

        if (hr < 0)
        {
            // Ensure that the IRestrictedErrorInfo is not left on the thread.
            hresult_error local_exception{ hr, take_ownership_from_abi };

            if (exception)
            {
                // Optionally transfer ownership to the caller.
                *exception = std::move(local_exception);
            }
        }

        return { result, take_ownership_from_abi };
    }

    template <> struct abi<Windows::Foundation::IActivationFactory>
    {
        struct WINRT_NOVTABLE type : inspectable_abi
        {
            virtual int32_t WINRT_CALL ActivateInstance(void** instance) noexcept = 0;
        };
    };

    template <> struct guid_storage<Windows::Foundation::IActivationFactory>
    {
        static constexpr guid value{ 0x00000035,0x0000,0x0000,{ 0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x46 } };
    };

    template <typename D> struct produce<D, Windows::Foundation::IActivationFactory> : produce_base<D, Windows::Foundation::IActivationFactory>
    {
        int32_t WINRT_CALL ActivateInstance(void** instance) noexcept final try
        {
            *instance = nullptr;
            typename D::abi_guard guard(this->shim());
            *instance = detach_abi(this->shim().ActivateInstance());
            return error_ok;
        }
        catch (...) { return to_hresult(); }
    };
}

namespace winrt
{
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

    template <typename Interface>
    impl::com_ref<Interface> create_instance(guid const& clsid, uint32_t context = 0x1 /*CLSCTX_INPROC_SERVER*/, void* outer = nullptr)
    {
        void* result;
        check_hresult(WINRT_CoCreateInstance(clsid, outer, context, guid_of<Interface>(), &result));
        return { result, take_ownership_from_abi };
    }

    namespace Windows::Foundation
    {
        struct IActivationFactory : IInspectable
        {
            IActivationFactory(std::nullptr_t = nullptr) noexcept {}
            IActivationFactory(void* ptr, take_ownership_from_abi_t) noexcept : IInspectable(ptr, take_ownership_from_abi) {}

            template <typename T>
            T ActivateInstance() const
            {
                IInspectable instance;
                check_hresult((*(impl::abi_t<IActivationFactory>**)this)->ActivateInstance(put_abi(instance)));
                return instance.try_as<T>();
            }
        };
    }
}
