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
            m_buffer.reserve(16 * 1024);
        }

        template <typename... Args>
        void write(std::string_view const& value, Args const&... args)
        {
            assert(count_placeholders(value) == sizeof...(Args));
            write_segment(value, args...);
        }

        void write(std::string_view const& value)
        {
            m_buffer.insert(m_buffer.end(), value.begin(), value.end());

#if defined(_DEBUG)
            if (debug_trace)
            {
                ::printf("%.*s", static_cast<int>(value.size()), value.data());
            }
#endif
        }

        void write(char const value)
        {
            m_buffer.push_back(value);

#if defined(_DEBUG)
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
            size_t const size = sprintf_s(buffer, format, args...);
            write(std::string_view{ buffer, size });
        }

        void flush_to_console() noexcept
        {
            printf("%.*s", static_cast<int>(m_buffer.size()), m_buffer.data());
            m_buffer.clear();
        }

        void flush_to_file(std::string_view const& filename)
        {
            winrt::file_handle file
            {
                CreateFileA(c_str(filename),
                    GENERIC_WRITE,
                    FILE_SHARE_READ,
                    nullptr, // default security
                    CREATE_ALWAYS,
                    FILE_ATTRIBUTE_NORMAL,
                    nullptr) // no template
            };

            if (!file)
            {
                winrt::throw_last_error();
            }

            DWORD copied = 0;

            std::array<uint8_t, 3> byte_order_mark{ 0xEF, 0xBB, 0xBF };

            winrt::check_bool(WriteFile(file.get(),
                byte_order_mark.data(),
                static_cast<uint32_t>(byte_order_mark.size()),
                &copied,
                nullptr));

            WINRT_ASSERT(copied == byte_order_mark.size());

            winrt::check_bool(WriteFile(file.get(),
                m_buffer.data(),
                static_cast<uint32_t>(m_buffer.size()),
                &copied,
                nullptr));

            WINRT_ASSERT(copied == m_buffer.size());
            m_buffer.clear();
        }

#if defined(_DEBUG)
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
            assert(offset != std::string_view::npos);
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
                        assert(false); // '@' placeholders are only for text.
                    }
                }

                write_segment(value.substr(offset + 1), rest...);
            }
        }

        std::vector<char> m_buffer;
    };

    template <auto F, typename... Args>
    auto bind(Args const&... args)
    {
        return [&](auto& writer)
        {
            F(writer, args...);
        };
    }

    template <auto F, typename Args>
    auto bind_each(Args const& args)
    {
        return [&](auto& writer)
        {
            for (auto&& arg : args)
            {
                F(writer, arg);
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
