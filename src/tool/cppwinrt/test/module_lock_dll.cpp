#include "catch.hpp"

// The default behavior (no macro) provides the static winrt::get_module_lock implementation for components/DLLs.

#include "winrt/base.h"

TEST_CASE("module_lock_dll")
{
    uint32_t const count = winrt::get_module_lock();

    ++winrt::get_module_lock();

    REQUIRE(winrt::get_module_lock() == count + 1);

    --winrt::get_module_lock();

    REQUIRE(winrt::get_module_lock() == count);
}
