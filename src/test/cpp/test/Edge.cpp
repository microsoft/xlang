#include "pch.h"
#include "winrt/Component.Edge.h"

using namespace winrt;
using namespace Component::Edge;

TEST_CASE("EmptyClass")
{
    EmptyClass c(nullptr);
}

TEST_CASE("StaticClass")
{
    StaticClass::StaticMethod();
}

TEST_CASE("ZeroClass")
{
    ZeroClass c;
    c.Method();
    ZeroClass::StaticMethod();
}

TEST_CASE("OneClass")
{
    OneClass c(1);
    REQUIRE(c.First() == 1);
    c.Method(2);

    REQUIRE(c.First() == 2);
    OneClass::StaticMethod(3);
    REQUIRE(c.First() == 3);
}

TEST_CASE("TwoClass")
{
    TwoClass c(1, 2);
    REQUIRE(c.First() == 1);
    REQUIRE(c.Second() == 2);

    c.Method(10, 20);
    REQUIRE(c.First() == 10);
    REQUIRE(c.Second() == 20);

    TwoClass::StaticMethod(100, 200);
    REQUIRE(c.First() == 100);
    REQUIRE(c.Second() == 200);
}

TEST_CASE("ThreeClass")
{
    ThreeClass c(1, 2, 3);
    REQUIRE(c.First() == 1);
    REQUIRE(c.Second() == 2);
    REQUIRE(c.Third() == 3);

    c.Method(10, 20, 30);
    REQUIRE(c.First() == 10);
    REQUIRE(c.Second() == 20);
    REQUIRE(c.Third() == 30);

    ThreeClass::StaticMethod(100, 200, 300);
    REQUIRE(c.First() == 100);
    REQUIRE(c.Second() == 200);
    REQUIRE(c.Third() == 300);
}