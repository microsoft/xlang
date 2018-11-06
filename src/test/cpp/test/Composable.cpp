#include "pch.h"
#include "winrt/Component.Composable.h"

using namespace winrt;
using namespace Component::Composable;

TEST_CASE("Composable.Base")
{
    Base base;
    REQUIRE(base.BaseMethod() == L"Base");
}

TEST_CASE("Composable.Derived")
{
    Derived derived;
    REQUIRE(derived.BaseMethod() == L"Base");
    REQUIRE(derived.DerivedMethod() == L"Derived");

    Base base = derived;
    REQUIRE(base.BaseMethod() == L"Base");

    derived = base.as<Derived>();
    REQUIRE(derived.DerivedMethod() == L"Derived");
}
