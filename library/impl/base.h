#pragma once

#include "winrt/base.h"
#include <bitset>
#include <variant>

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
