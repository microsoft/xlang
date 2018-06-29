
#if WINRT_PLATFORM_WINDOWS
extern "C"
{
    int32_t WINRT_CALL WINRT_GetFileSizeEx(void* hFile, int64_t * lpFileSize);
    int32_t WINRT_CALL WINRT_UnmapViewOfFile(void const* baseAddress);
    void *  WINRT_CALL WINRT_CreateFileA(
        char const* lpFileName,
        uint32_t dwDesiredAccess,
        uint32_t  dwShareMode,
        void * lpSecurityAttributes,
        uint32_t dwCreationDisposition,
        uint32_t dwFlagsAndAttributes,
        void * hTemplateFile);
    void * WINRT_CALL WINRT_CreateFileMappingA(
        void * hFile,
        void * lpFileMappingAttributes,
        uint32_t flProtect,
        uint32_t dwMaximumSizeHigh,
        uint32_t dwMaximumSizeLow,
        char const* lpName);
    void * WINRT_CALL WINRT_MapViewOfFile(
        void * hFileMappingObject,
        uint32_t dwDesiredAccess,
        uint32_t dwFileOffsetHigh,
        uint32_t dwFileOffsetLow,
#ifdef _WIN64
        uint64_t dwNumberOfBytesToMap); 
#else
        uint32_t dwNumberOfBytesToMap); 
#endif
}

WINRT_LINK(UnmapViewOfFile, 4);
WINRT_LINK(CreateFileA, 28);
WINRT_LINK(GetFileSizeEx, 8);
WINRT_LINK(CreateFileMappingA, 24);
WINRT_LINK(MapViewOfFile, 20);

#else
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#endif

namespace xlang::meta::reader
{
    template <typename T>
    struct range : std::pair<T, T>
    {
        explicit range(std::pair<T, T>&& values) : std::pair<T, T>{ std::move(values) }
        {
        }

        auto begin() const noexcept
        {
            return this->first;
        }

        auto end() const noexcept
        {
            return this->second;
        }
    };

    template <typename Container, typename T>
    auto equal_range(Container const& container, T const& value) noexcept
    {
        return range{ std::equal_range(container.begin(), container.end(), value) };
    }

    template <typename Container, typename T, typename Compare>
    auto equal_range(Container const& container, T const& value, Compare compare) noexcept
    {
        return range{ std::equal_range(container.begin(), container.end(), value, compare) };
    }

    struct byte_view
    {
        byte_view() noexcept = default;

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
        auto const& as(uint32_t const offset = 0) const
        {
            check_available(offset + sizeof(T));
            return reinterpret_cast<T const&>(*(m_first + offset));
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
                throw_invalid(u"Buffer too small");
            }
        }

        uint8_t const* m_first{};
        uint8_t const* m_last{};
    };

    struct file_view : byte_view
    {
        file_view(file_view const&) = delete;
        file_view& operator=(file_view const&) = delete;

        file_view(std::string_view const& path) : byte_view{ open_file(path) }
        {
        }

        ~file_view() noexcept
        {
#if WINRT_PLATFORM_WINDOWS
            WINRT_VERIFY(WINRT_UnmapViewOfFile(begin()));
#else
            WINRT_VERIFY(munmap(const_cast<void*>(reinterpret_cast<void const*>(begin())), size()) == 0);
#endif
        }

    private:

        static byte_view open_file(std::string_view const& path)
        {
#if WINRT_PLATFORM_WINDOWS
            winrt::file_handle file{ WINRT_CreateFileA(c_str(path), 0x80000000L /*GENERIC_READ*/, 0x00000001 /*FILE_SHARE_READ*/, nullptr, 3 /*OPEN_EXISTING*/, 0x00000080 /*FILE_ATTRIBUTE_NORMAL*/, nullptr) };

            if (!file)
            {
                winrt::throw_last_error();
            }

            int64_t size{};
            WINRT_VERIFY(WINRT_GetFileSizeEx(file.get(), &size));

            if (!size)
            {
                return{};
            }

            winrt::handle mapping{ WINRT_CreateFileMappingA(file.get(), nullptr, 0x02 /*PAGE_READONLY*/, 0, 0, nullptr) };

            if (!mapping)
            {
                winrt::throw_last_error();
            }

            auto const first{ static_cast<uint8_t const*>(WINRT_MapViewOfFile(mapping.get(), 0x0004 /*FILE_MAP_READ*/, 0, 0, 0)) };
            return{ first, first + size };
#else
            struct stat st;
            if (stat(c_str(path), &st) != 0)
            {
                // TODO: Check errno
                winrt::throw_hresult(winrt::impl::error_fail);
            }

            int fd = open(c_str(path), O_RDONLY, 0);
            if (fd <= 0)
            {
                // TODO: Check errno
                winrt::throw_hresult(winrt::impl::error_fail);
            }

            auto const first = static_cast<uint8_t const*>(mmap(nullptr, st.st_size, PROT_READ, MAP_PRIVATE | MAP_POPULATE, fd, 0));
            if (first == MAP_FAILED)
            {
                // TODO: Check errno
                winrt::throw_hresult(winrt::impl::error_fail);
            } 

            return{ first, first + st.st_size };
#endif
        }
    };
}
