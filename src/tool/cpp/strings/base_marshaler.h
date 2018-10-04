
namespace winrt::impl
{
    inline int32_t make_marshaler(IUnknown* outer, void** result) noexcept
    {
        struct marshaler final : IMarshal
        {
            marshaler(IUnknown* object) noexcept
            {
                m_object.copy_from(object);
            }

            int32_t WINRT_CALL QueryInterface(guid const& id, void** object) noexcept final
            {
                if (is_guid_of<IMarshal>(id))
                {
                    *object = static_cast<IMarshal*>(this);
                    AddRef();
                    return error_ok;
                }

                return m_object->QueryInterface(id, object);
            }

            uint32_t WINRT_CALL AddRef() noexcept final
            {
                return 1 + m_references.fetch_add(1, std::memory_order_relaxed);
            }

            uint32_t WINRT_CALL Release() noexcept final
            {
                uint32_t const remaining = m_references.fetch_sub(1, std::memory_order_relaxed) - 1;

                if (remaining == 0)
                {
                    delete this;
                }

                return remaining;
            }

            int32_t WINRT_CALL GetUnmarshalClass(guid const& riid, void* pv, uint32_t dwDestContext, void* pvDestContext, uint32_t mshlflags, guid* pCid) noexcept final
            {
                if (m_marshaler)
                {
                    return m_marshaler->GetUnmarshalClass(riid, pv, dwDestContext, pvDestContext, mshlflags, pCid);
                }

                return error_bad_alloc;
            }

            int32_t WINRT_CALL GetMarshalSizeMax(guid const& riid, void* pv, uint32_t dwDestContext, void* pvDestContext, uint32_t mshlflags, uint32_t* pSize) noexcept final
            {
                if (m_marshaler)
                {
                    return m_marshaler->GetMarshalSizeMax(riid, pv, dwDestContext, pvDestContext, mshlflags, pSize);
                }

                return error_bad_alloc;
            }

            int32_t WINRT_CALL MarshalInterface(void* pStm, guid const& riid, void* pv, uint32_t dwDestContext, void* pvDestContext, uint32_t mshlflags) noexcept final
            {
                if (m_marshaler)
                {
                    return m_marshaler->MarshalInterface(pStm, riid, pv, dwDestContext, pvDestContext, mshlflags);
                }

                return error_bad_alloc;
            }

            int32_t WINRT_CALL UnmarshalInterface(void* pStm, guid const& riid, void** ppv) noexcept final
            {
                if (m_marshaler)
                {
                    return m_marshaler->UnmarshalInterface(pStm, riid, ppv);
                }

                *ppv = nullptr;
                return error_bad_alloc;
            }

            int32_t WINRT_CALL ReleaseMarshalData(void* pStm) noexcept final
            {
                if (m_marshaler)
                {
                    return m_marshaler->ReleaseMarshalData(pStm);
                }

                return error_bad_alloc;
            }

            int32_t WINRT_CALL DisconnectObject(uint32_t dwReserved) noexcept final
            {
                if (m_marshaler)
                {
                    return m_marshaler->DisconnectObject(dwReserved);
                }

                return error_bad_alloc;
            }

        private:

            static com_ptr<IMarshal> get_marshaler() noexcept
            {
                com_ptr<IUnknown> unknown;
                WINRT_VERIFY_(error_ok, WINRT_CoCreateFreeThreadedMarshaler(nullptr, unknown.put_void()));
                return unknown ? unknown.try_as<IMarshal>() : nullptr;
            }

            com_ptr<IUnknown> m_object;
            com_ptr<IMarshal> m_marshaler{ get_marshaler() };
            std::atomic<uint32_t> m_references{ 1 };
        };

        *result = new (std::nothrow) marshaler(outer);
        return *result ? error_ok : error_bad_alloc;
    }
}
