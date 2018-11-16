
WINRT_EXPORT namespace xlang
{
    template <typename T>
    struct weak_ref
    {
        weak_ref(std::nullptr_t = nullptr) noexcept {}

        weak_ref(impl::com_ref<T> const& object)
        {
            if (object)
            {
                if constexpr(impl::is_implements_v<T>)
                {
                    m_ref = std::move(object->get_weak().m_ref);
                }
                else
                {
                    check_hresult(object.template as<impl::IWeakReferenceSource>()->GetWeakReference(m_ref.put()));
                }
            }
        }

        auto get() const noexcept
        {
            impl::com_ref<T> object{ nullptr };

            if (m_ref)
            {
                if constexpr(impl::is_implements_v<T>)
                {
                    impl::com_ref<default_interface<T>> temp;
                    m_ref->Resolve(guid_of<T>(), put_abi(temp));
                    attach_abi(object, get_self<T>(temp));
                    detach_abi(temp);
                }
                else
                {
                    m_ref->Resolve(guid_of<T>(), put_abi(object));
                }
            }

            return object;
        }

        auto put() noexcept
        {
            return m_ref.put();
        }

        explicit operator bool() const noexcept
        {
            return static_cast<bool>(m_ref);
        }

    private:

        com_ptr<impl::IWeakReference> m_ref;
    };

    template <typename T>
    weak_ref<impl::wrapped_type_t<T>> make_weak(T const& object)
    {
        return object;
    }
}
