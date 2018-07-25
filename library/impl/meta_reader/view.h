
namespace xlang::meta::reader
{
    template <typename T>
    auto const& begin(std::pair<T, T> const& values) noexcept
    {
        return values.first;
    }

    template <typename T>
    auto const& end(std::pair<T, T> const& values) noexcept
    {
        return values.second;
    }

    template <typename Container, typename T>
    auto equal_range(Container const& container, T const& value) noexcept
    {
        return std::equal_range(container.begin(), container.end(), value);
    }

    template <typename Container, typename T, typename Compare>
    auto equal_range(Container const& container, T const& value, Compare compare) noexcept
    {
        return std::equal_range(container.begin(), container.end(), value, compare);
    }

    struct byte_view;
    inline int32_t uncompress_signed(byte_view& cursor, uint32_t length);

    struct byte_view
    {
        byte_view() noexcept = default;
        byte_view(byte_view const&) noexcept = default;
        byte_view& operator=(byte_view const&) noexcept = default;

        byte_view(byte_view&& other) noexcept :
            m_first(std::exchange(other.m_first, {})),
            m_last(std::exchange(other.m_last, {}))
        {
        }

        byte_view& operator=(byte_view&& other) noexcept
        {
            m_first = std::exchange(other.m_first, {});
            m_last = std::exchange(other.m_last, {});
            return *this;
        }

        byte_view(uint8_t const* const first, uint8_t const* const last) noexcept :
            m_first(first),
            m_last(last)
        {
        }

        auto begin() const noexcept
        {
            return m_first;
        }

        auto end() const noexcept
        {
            return m_last;
        }

        uint32_t size() const noexcept
        {
            return static_cast<uint32_t>(end() - begin());
        }

        explicit operator bool() const noexcept
        {
            return size() > 0;
        }

        byte_view seek(uint32_t const offset) const
        {
            check_available(offset);
            return{ m_first + offset, m_last };
        }

        byte_view sub(uint32_t const offset, uint32_t const size) const
        {
            check_available(offset + size);
            return{ m_first + offset, m_first + offset + size };
        }

        template <typename T>
        T const& as(uint32_t const offset = 0) const
        {
            check_available(offset + sizeof(T));
            return reinterpret_cast<T const&>(*(m_first + offset));
        }

        std::string_view as_string(uint32_t const offset = 0) const
        {
            static_assert(sizeof(uint8_t) == 1);
            check_available(offset + 1);
            auto const length = as<uint8_t>(offset);
            if (length == 0)
            {
                return "";
            }
            else if (length == 0xff)
            {
                return { nullptr, 0 };
            }
            else
            {
                check_available(offset + 1 + length);
                return { reinterpret_cast<char const*>(m_first + offset + 1), length };
            }
        }

        template <typename T>
        auto as_array(uint32_t const offset, uint32_t const count) const
        {
            check_available(offset + count * sizeof(T));
            return reinterpret_cast<T const*>(m_first + offset);
        }

    private:

        void check_available(uint32_t const offset) const
        {
            if (m_first + offset > m_last)
            {
                throw_invalid("Buffer too small");
            }
        }

        uint8_t const* m_first{};
        uint8_t const* m_last{};
    };

    struct file_view : byte_view
    {
        file_view(file_view const&) = delete;
        file_view& operator=(file_view const&) = delete;
        file_view(file_view&&) noexcept = default;
        file_view& operator=(file_view&&) noexcept = default;

        file_view(std::string_view const& path) : byte_view{ open_file(path) }
        {
        }

        ~file_view() noexcept
        {
            if (*this)
            {
                WINRT_VERIFY(UnmapViewOfFile(begin()));
            }
        }

    private:

        static byte_view open_file(std::string_view const& path)
        {
            winrt::file_handle file{ CreateFileA(c_str(path), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr) };

            if (!file)
            {
                winrt::throw_last_error();
            }

            LARGE_INTEGER size{};
            WINRT_VERIFY(GetFileSizeEx(file.get(), &size));

            if (!size.QuadPart)
            {
                return{};
            }

            winrt::handle mapping{ CreateFileMappingA(file.get(), nullptr, PAGE_READONLY, 0, 0, nullptr) };

            if (!mapping)
            {
                winrt::throw_last_error();
            }

            auto const first{ static_cast<uint8_t const*>(MapViewOfFile(mapping.get(), FILE_MAP_READ, 0, 0, 0)) };
            return{ first, first + size.QuadPart };
        }
    };
}
