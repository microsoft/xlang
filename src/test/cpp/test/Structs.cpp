#include "pch.h"
#include "winrt/Component.Structs.h"

using namespace winrt;
using namespace Component::Structs;

TEST_CASE("Structs")
{
    Simple a{ 1, 2, 3, 4 };

    Simple b{};
    b.A = 1;
    b.R = 2;
    b.G = 3;
    b.B = 4;

    REQUIRE(a == b);
    REQUIRE_FALSE(a != b);
}
