#include "pch.h"
#include "winrt/test_component.h"

using namespace winrt;
using namespace test_component;

TEST_CASE("delegates")
{
    {
        bool run{};
        AgileDelegate d = [&] {run = true; };
        REQUIRE(!run);
        d();
        REQUIRE(run);
    }
    {
        hstring value;
        InDelegate d = [&](hstring const& in)
        {
            value = in;
        };
        REQUIRE(value.empty());
        d(L"Test");
        REQUIRE(value == L"Test");
    }
    {
        ReturnDelegate d = [] {return L"Test"; };
        REQUIRE(d() == L"Test");
    }
    {
        OutDelegate d = [](hstring& value)
        {
            value = L"Test";
        };
        hstring value;
        d(value);
        REQUIRE(value == L"Test");
    }
}
