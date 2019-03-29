
namespace winrt::param
{
    struct hstring
    {
        hstring() noexcept : m_handle(nullptr) {}
        hstring(hstring const& values) = delete;
        hstring& operator=(hstring const& values) = delete;
        hstring(std::nullptr_t) = delete;

		~hstring() noexcept
		{
			xlang_delete_string(m_handle);
		}

        hstring(winrt::hstring const& value) noexcept : m_handle(get_abi(value))
        {
        }

		template <typename char_type, typename = std::enable_if_t<impl::is_char_type_supported<char_type>::value>>
		hstring(std::basic_string_view<char_type> const& value) noexcept
		{
			init<false, char_type>(value);
		}

		template <typename char_type, typename = std::enable_if_t<impl::is_char_type_supported<char_type>::value>>
		hstring(std::basic_string<char_type> const& value) noexcept
		{
			init<true, char_type>(value);
		}

		template <typename char_type, typename = std::enable_if_t<impl::is_char_type_supported<char_type>::value>>
        hstring(char_type const* const value) noexcept
        {
			init<true, char_type>(value);
		}

        operator winrt::hstring const&() const noexcept
        {
            return *reinterpret_cast<winrt::hstring const*>(this);
        }

    private:
		template <typename char_type, bool is_safe>
		void init(std::basic_string_view<char_type> str) noexcept
		{
			static_assert(impl::is_char_type_supported<char_type>::value);
			auto const value = normalize_char_type(str);
			auto const length = static_cast<uint32_t>(value.size());
			if constexpr (is_safe)
			{
				if constexpr (sizeof(char_type) == sizeof(xlang_char8))
				{
					WINRT_VERIFY_(nullptr, xlang_create_string_reference_utf8(str.data(), length, &m_header, &m_handle));
				}
				else
				{
					WINRT_VERIFY_(nullptr, xlang_create_string_reference_utf16(str.data(), length, &m_header, &m_handle));
				}
			}
			else
			{
				if constexpr (sizeof(char_type) == sizeof(xlang_char8))
				{
					if (nullptr != xlang_create_string_reference_utf8(str.data(), length, &m_header, &m_handle))
					{
						std::terminate();
					}
				}
				else
				{
					if (nullptr != xlang_create_string_reference_utf16(str.data(), length, &m_header, &m_handle))
					{
						std::terminate();
					}
				}
			}
		}

        xlang_string m_handle;
		xlang_string_header m_header;
    };

    inline xlang_string get_abi(hstring const& object) noexcept
    {
        return *(xlang_string*)(&object);
    }
}

namespace winrt::impl
{
    template <typename T>
    using param_type = std::conditional_t<std::is_same_v<T, hstring>, param::hstring, T>;
}
