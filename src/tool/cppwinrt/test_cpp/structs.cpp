#include "pch.h"
#include "winrt/test_component.Structs.Nested.h"

using namespace winrt;
using namespace test_component;

TEST_CASE("structs")
{
    Structs::Nested::Outer outer{};
    outer.Depends.InnerValue = 1;
    outer.OuterValue = 2;
}
