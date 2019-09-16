#include "catch.hpp"

// Defining WINRT_CUSTOM_MODULE_LOCK means you need to provide your own winrt::get_module_lock implementation.
// This can be useful if you have some custom hosting environment that does not use DllCanUnloadNow.

#define WINRT_CUSTOM_MODULE_LOCK

namespace winrt
{
    inline auto get_module_lock() noexcept
    {
        struct lock
        {
            uint32_t operator++() noexcept
            {
                return 10;
            }

            uint32_t operator--() noexcept
            {
                return 1;
            }

            operator uint32_t() const noexcept
            {
                return 123;
            }
        };

        return lock{};
    }
}

#include "winrt/base.h"

TEST_CASE("module_lock_custom")
{
    REQUIRE(++winrt::get_module_lock() == 10);

    REQUIRE(--winrt::get_module_lock() == 1);

    REQUIRE(winrt::get_module_lock() == 123);
}
