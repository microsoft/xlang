#include "pch.h"
#include "winrt/test_component_fast.h"

using namespace winrt::test_component_fast;

TEST_CASE("Simple")
{
    Simple c;
    REQUIRE(c.Method1() == L"Method1");
    REQUIRE(c.Method2() == L"Method2");
    REQUIRE(c.Method3() == L"Method3");
}
