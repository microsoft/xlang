
namespace winrt::impl
{
    template <typename T, typename H>
    struct implements_delegate : abi_t<T>, H
    {
        implements_delegate(H&& handler) : H(std::forward<H>(handler)) {}

        int32_t WINRT_CALL QueryInterface(guid const& id, void** result) noexcept final
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

        uint32_t WINRT_CALL AddRef() noexcept final
        {
            return 1 + m_references.fetch_add(1, std::memory_order_relaxed);
        }

        uint32_t WINRT_CALL Release() noexcept final
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

    template <typename T, typename H>
    T make_delegate(H&& handler)
    {
        return { static_cast<void*>(static_cast<abi_t<T>*>(new delegate_t<T, H>(std::forward<H>(handler)))), take_ownership_from_abi };
    }

    template <typename... T>
    struct WINRT_NOVTABLE variadic_delegate_abi : unknown_abi
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

        int32_t WINRT_CALL QueryInterface(guid const& id, void** result) noexcept final
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

        uint32_t WINRT_CALL AddRef() noexcept final
        {
            return 1 + m_references.fetch_add(1, std::memory_order_relaxed);
        }

        uint32_t WINRT_CALL Release() noexcept final
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

WINRT_EXPORT namespace winrt
{
    template <typename... T>
    struct WINRT_EBO delegate : Windows::Foundation::IUnknown
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

        void operator()(T const&... args) const
        {
            (*(impl::variadic_delegate_abi<T...>**)this)->invoke(args...);
        }

    private:

        template <typename H>
        static winrt::delegate<T...> make(H&& handler)
        {
            return { new impl::variadic_delegate<H, T...>(std::forward<H>(handler)), take_ownership_from_abi };
        }
    };
}
