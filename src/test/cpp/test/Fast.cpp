#include "pch.h"
#include "winrt/Component.Fast.h"

using namespace winrt;
using namespace Component::Fast;

TEST_CASE("SlowClass")
{
    SlowClass c;
    REQUIRE(c.First() == L"Slow.First");
    REQUIRE(c.Second() == L"Slow.Second");
    REQUIRE(c.Third() == L"Slow.Third");
    REQUIRE(c.Fourth() == L"Slow.Fourth");
    REQUIRE(c.NotExclusive() == L"Slow.NotExclusive");
    REQUIRE(SlowClass::StaticMethod() == L"Slow.Static");
}

TEST_CASE("FastClass")
{
    FastClass c;
    REQUIRE(c.First() == L"Fast.First");
    REQUIRE(c.Second() == L"Fast.Second");
    REQUIRE(c.Third() == L"Fast.Third");
    REQUIRE(c.Fourth() == L"Fast.Fourth");
    REQUIRE(c.NotExclusive() == L"Fast.NotExclusive");
    REQUIRE(FastClass::StaticMethod() == L"Fast.Static");
}
