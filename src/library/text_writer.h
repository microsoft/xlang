#pragma once

#include "impl/base.h"

namespace xlang::text
{
    template <typename T>
    struct writer_base
    {
        writer_base(writer_base const&) = delete;
        writer_base& operator=(writer_base const&) = delete;

        writer_base()
        {
            m_first.reserve(16 * 1024);
        }

        template <typename... Args>
        void write(std::string_view const& value, Args const&... args)
        {
#if defined(XLANG_DEBUG)
            auto expected = count_placeholders(value);
            auto actual = sizeof...(Args);
            XLANG_ASSERT(expected == actual);
#endif
            write_segment(value, args...);
        }

        template <typename... Args>
        [[nodiscard]] std::string write_temp(std::string_view const& value, Args const&... args)
        {
#if defined(XLANG_DEBUG)
            bool restore_debug_trace = debug_trace;
            debug_trace = false;
#endif
            auto const size = m_first.size();

            XLANG_ASSERT(count_placeholders(value) == sizeof...(Args));
            write_segment(value, args...);

            std::string result{ m_first.data() + size, m_first.size() - size };
            m_first.resize(size);

#if defined(XLANG_DEBUG)
            debug_trace = restore_debug_trace;
#endif
            return result;
        }

        void write(std::string_view const& value)
        {
            m_first.insert(m_first.end(), value.begin(), value.end());

#if defined(XLANG_DEBUG)
            if (debug_trace)
            {
                ::printf("%.*s", static_cast<int>(value.size()), value.data());
            }
#endif
        }

        void write(char const value)
        {
            m_first.push_back(value);

#if defined(XLANG_DEBUG)
            if (debug_trace)
            {
                ::printf("%c", value);
            }
#endif
        }

        void write_code(std::string_view const& value)
        {
            write(value);
        }

        template <typename F, typename = std::enable_if_t<std::is_invocable_v<F, T&>>>
        void write(F const& f)
        {
            f(*static_cast<T*>(this));
        }

        void write(int32_t const value)
        {
            write(std::to_string(value));
        }

        void write(int64_t const value)
        {
            write(std::to_string(value));
        }

        void write(uint64_t const value)
        {
            write(std::to_string(value));
        }

        template <typename... Args>
        void write_printf(char const* format, Args const&... args)
        {
            char buffer[128];
#if XLANG_PLATFORM_WINDOWS
            size_t const size = sprintf_s(buffer, format, args...);
#else
            size_t const size = snprintf(buffer, sizeof(buffer), format, args...);
#endif
            write(std::string_view{ buffer, size });
        }

        template <auto F, typename List, typename... Args>
        void write_each(List const& list, Args const&... args)
        {
            for (auto&& item : list)
            {
                F(*static_cast<T*>(this), item, args...);
            }
        }
        
        void swap() noexcept
        {
            std::swap(m_second, m_first);
        }

        void flush_to_console() noexcept
        {
            printf("%.*s", static_cast<int>(m_first.size()), m_first.data());
            printf("%.*s", static_cast<int>(m_second.size()), m_second.data());
            m_first.clear();
            m_second.clear();
        }

        void flush_to_file(std::string const& filename)
        {
            std::ofstream file{ filename, std::ios::out | std::ios::binary };
            std::array<uint8_t, 3> bom{ 0xEF, 0xBB, 0xBF };
            file.write(reinterpret_cast<char*>(bom.data()), bom.size());
            file.write(m_first.data(), m_first.size());
            file.write(m_second.data(), m_second.size());
            m_first.clear();
            m_second.clear();
        }

        void flush_to_file(std::experimental::filesystem::path const& filename)
        {
            flush_to_file(filename.string());
        }

        char back()
        {
            return m_first.back();
        }

#if defined(XLANG_DEBUG)
        bool debug_trace{};
#endif

    private:

        static constexpr uint32_t count_placeholders(std::string_view const& format) noexcept
        {
            uint32_t count{};
            bool escape{};

            for (auto c : format)
            {
                if (c == '^')
                {
                    escape = true;
                    continue;
                }

                if ((c == '%' || c == '@') && !escape)
                {
                    ++count;
                }

                escape = false;
            }

            return count;
        }

        void write_segment(std::string_view const& value)
        {
            write(value);
        }

        template <typename First, typename... Rest>
        void write_segment(std::string_view const& value, First const& first, Rest const&... rest)
        {
            auto offset = value.find_first_of("^%@");
            XLANG_ASSERT(offset != std::string_view::npos);
            write(value.substr(0, offset));

            if (value[offset] == '^')
            {
                auto next = value[offset + 1];

                if (next == '%' || next == '@')
                {
                    write(next);
                    offset++;
                }
                else
                {
                    write('^');
                }

                write_segment(value.substr(offset + 1), first, rest...);
            }
            else
            {
                if (value[offset] == '%')
                {
                    static_cast<T*>(this)->write(first);
                }
                else
                {
                    if constexpr (std::is_convertible_v<First, std::string_view>)
                    {
                        static_cast<T*>(this)->write_code(first);
                    }
                    else
                    {
                        XLANG_ASSERT(false); // '@' placeholders are only for text.
                    }
                }

                write_segment(value.substr(offset + 1), rest...);
            }
        }

        std::vector<char> m_second;
        std::vector<char> m_first;
    };

    template <auto F, typename... Args>
    auto bind(Args&&... args)
    {
        return [&](auto& writer)
        {
            F(writer, args...);
        };
    }

    template <auto F, typename List, typename... Args>
    auto bind_each(List const& list, Args const&... args)
    {
        return [&](auto& writer)
        {
            for (auto&& item : list)
            {
                F(writer, item, args...);
            }
        };
    }

    template <auto F, typename T>
    auto bind_list(std::string_view const& delimiter, T const& list)
    {
        return [&](auto& writer)
        {
            bool first{ true };

            for (auto&& item : list)
            {
                if (first)
                {
                    first = false;
                }
                else
                {
                    writer.write(delimiter);
                }

                F(writer, item);
            }
        };
    }

    template <typename T>
    auto bind_list(std::string_view const& delimiter, T const& list)
    {
        return [&](auto& writer)
        {
            bool first{ true };

            for (auto&& item : list)
            {
                if (first)
                {
                    first = false;
                }
                else
                {
                    writer.write(delimiter);
                }

                writer.write(item);
            }
        };
    }
}
