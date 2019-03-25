#include "pch.h"
#include "winrt/test_component.fast.h"

using namespace winrt;
using namespace test_component::Fast;

TEST_CASE("fastabi")
{
    Class c;

    c.Class_1();
    c.Class_2();
    c.Class_3();
}
