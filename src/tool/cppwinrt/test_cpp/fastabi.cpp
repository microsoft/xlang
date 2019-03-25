#include "pch.h"
#include "winrt/test_component.fast.h"

using namespace winrt;
using namespace test_component::Fast;

namespace
{
    bool test()
    {
        Class c;

        return 
            c.Class_1() == L"one" &&
            c.Class_2() == L"two" &&
            c.Class_3() == L"three";
    }
}

TEST_CASE("fastabi")
{
    REQUIRE(test());
}
