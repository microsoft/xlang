
#include "winrt/base.h"
#include <bitset>
#include <variant>

namespace xlang
{
    using namespace std::literals;

    [[noreturn]] inline void throw_invalid(std::u16string_view const& message)
    {
        throw winrt::hresult_invalid_argument(message);
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
