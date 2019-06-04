
namespace winrt::impl
{
    template <typename T, typename H>
    struct implements_delegate : abi_t<T>, H
    {
        implements_delegate(H&& handler) : H(std::forward<H>(handler)) {}

        int32_t WINRT_IMPL_CALL QueryInterface(guid const& id, void** result) noexcept final
        {
            if (is_guid_of<T>(id) || is_guid_of<Windows::Foundation::IUnknown>(id) || is_guid_of<IAgileObject>(id))
            {
                *result = static_cast<abi_t<T>*>(this);
                AddRef();
                return error_ok;
            }

            if (is_guid_of<IMarshal>(id))
            {
                return make_marshaler(this, result);
            }

            *result = nullptr;
            return error_no_interface;
        }

        uint32_t WINRT_IMPL_CALL AddRef() noexcept final
        {
            return 1 + m_references.fetch_add(1, std::memory_order_relaxed);
        }

        uint32_t WINRT_IMPL_CALL Release() noexcept final
        {
            uint32_t const target = m_references.fetch_sub(1, std::memory_order_release) - 1;

            if (target == 0)
            {
                std::atomic_thread_fence(std::memory_order_acquire);
                delete static_cast<delegate<T, H>*>(this);
            }

            return target;
        }

    private:

        std::atomic<uint32_t> m_references{ 1 };
    };

    template <typename T, typename H>
    T make_delegate(H&& handler)
    {
        return { static_cast<void*>(static_cast<abi_t<T>*>(new delegate<T, H>(std::forward<H>(handler)))), take_ownership_from_abi };
    }

    template <typename T>
    T make_agile_delegate(T const& delegate) noexcept
    {
        if constexpr (!has_category_v<T>)
        {
            return delegate;
        }
        else
        {
            if (delegate.template try_as<IAgileObject>())
            {
                return delegate;
            }

            com_ptr<IAgileReference> ref;
            WINRT_RoGetAgileReference(0, guid_of<T>(), get_abi(delegate), ref.put_void());

            if (ref)
            {
                return[ref = std::move(ref)](auto&&... args)
                {
                    T delegate;
                    ref->Resolve(guid_of<T>(), put_abi(delegate));
                    return delegate(args...);
                };
            }

            return delegate;
        }
    }

    template <typename... T>
    struct WINRT_IMPL_NOVTABLE variadic_delegate_abi : unknown_abi
    {
        virtual void invoke(T const&...) = 0;
    };

    template <typename H, typename... T>
    struct variadic_delegate final : variadic_delegate_abi<T...>, H
    {
        variadic_delegate(H&& handler) : H(std::forward<H>(handler)) {}

        void invoke(T const&... args) final
        {
            (*this)(args...);
        }

        int32_t WINRT_IMPL_CALL QueryInterface(guid const& id, void** result) noexcept final
        {
            if (is_guid_of<Windows::Foundation::IUnknown>(id) || is_guid_of<IAgileObject>(id))
            {
                *result = static_cast<unknown_abi*>(this);
                AddRef();
                return error_ok;
            }

            *result = nullptr;
            return error_no_interface;
        }

        uint32_t WINRT_IMPL_CALL AddRef() noexcept final
        {
            return 1 + m_references.fetch_add(1, std::memory_order_relaxed);
        }

        uint32_t WINRT_IMPL_CALL Release() noexcept final
        {
            uint32_t const target = m_references.fetch_sub(1, std::memory_order_release) - 1;

            if (target == 0)
            {
                std::atomic_thread_fence(std::memory_order_acquire);
                delete this;
            }

            return target;
        }

    private:

        std::atomic<uint32_t> m_references{ 1 };
    };
}

namespace winrt
{
    template <typename... T>
    struct WINRT_IMPL_EBO delegate : Windows::Foundation::IUnknown
    {
        delegate(std::nullptr_t = nullptr) noexcept {}
        delegate(void* ptr, take_ownership_from_abi_t) noexcept : IUnknown(ptr, take_ownership_from_abi) {}

        template <typename L>
        delegate(L handler) :
            delegate(make(std::forward<L>(handler)))
        {}

        template <typename F> delegate(F* handler) :
            delegate([=](auto&&... args) { handler(args...); })
        {}

        template <typename O, typename M> delegate(O* object, M method) :
            delegate([=](auto&&... args) { ((*object).*(method))(args...); })
        {}

        template <typename O, typename M> delegate(com_ptr<O>&& object, M method) :
            delegate([o = std::move(object), method](auto&&... args) { return ((*o).*(method))(args...); })
        {
        }

        template <typename O, typename M> delegate(weak_ref<O>&& object, M method) :
            delegate([o = std::move(object), method](auto&&... args) { if (auto s = o.get()) { ((*s).*(method))(args...); } })
        {
        }

        void operator()(T const&... args) const
        {
            (*(impl::variadic_delegate_abi<T...>**)this)->invoke(args...);
        }

    private:

        template <typename H>
        static winrt::delegate<T...> make(H&& handler)
        {
            return { static_cast<void*>(new impl::variadic_delegate<H, T...>(std::forward<H>(handler))), take_ownership_from_abi };
        }
    };
}
