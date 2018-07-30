#pragma once

#if defined(_WIN32)
#define XLANG_PLATFORM_WINDOWS 1
#else 
#define XLANG_PLATFORM_WINDOWS 0
#endif

#if XLANG_PLATFORM_WINDOWS 
#define NOMINMAX
#include <windows.h>
#else
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#endif

#include <stdexcept>
#include <assert.h>
#include <array>
#include <bitset>
#include <fstream>
#include <future>
#include <list>
#include <map>
#include <optional>
#include <string_view>
#include <variant>
#include <vector>

#define XLANG_ASSERT assert

namespace xlang
{
    using namespace std::literals;

    [[noreturn]] inline void throw_invalid(std::string const& message)
    {
        throw std::invalid_argument(message);
    }

    template <typename...T>
    [[noreturn]] inline void throw_invalid(std::string message, T const&... args)
    {
        (message.append(args), ...);
        throw std::invalid_argument(message);
    }

    template <typename T>
    auto c_str(std::basic_string_view<T> const& view) noexcept
    {
        if (*(view.data() + view.size()))
        {
            std::terminate();
        }

        return view.data();
    }
}
