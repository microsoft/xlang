
WINRT_EXPORT namespace winrt::param
{
    struct hstring
    {
        hstring() noexcept : m_handle(nullptr) {}
        hstring(hstring const& values) = delete;
        hstring& operator=(hstring const& values) = delete;
        hstring(std::nullptr_t) = delete;

        hstring(winrt::hstring const& value) noexcept : m_handle(get_abi(value))
        {
        }

        hstring(std::wstring_view const& value) noexcept
        {
            if (impl::error_ok != WINRT_WindowsCreateStringReference(value.data(), static_cast<uint32_t>(value.size()), &m_header, &m_handle))
            {
                std::terminate();
            }
        }

        hstring(std::wstring const& value) noexcept
        {
            WINRT_VERIFY_(impl::error_ok, WINRT_WindowsCreateStringReference(value.data(), static_cast<uint32_t>(value.size()), &m_header, &m_handle));
        }

        hstring(wchar_t const* const value) noexcept
        {
            WINRT_VERIFY_(impl::error_ok, WINRT_WindowsCreateStringReference(value, static_cast<uint32_t>(wcslen(value)), &m_header, &m_handle));
        }

    private:

        struct header
        {
            union
            {
                void* Reserved1;
#ifdef _WIN64
                char Reserved2[24];
#else
                char Reserved2[20];
#endif
            } Reserved;
        };
        
        void* m_handle;
        header m_header;
    };

    inline void* get_abi(hstring const& object) noexcept
    {
        return *(void**)(&object);
    }
}

namespace winrt::impl
{
    template <typename T>
    using param_type = std::conditional_t<std::is_same_v<T, hstring>, param::hstring, T>;
}
