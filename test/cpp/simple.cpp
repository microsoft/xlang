#include "pch.h"
#include "winrt/Simple.h"

using namespace winrt;

struct Implementation : implements<Implementation, Simple::IStringable>
{
    hstring ToString()
    {
        return L"Simple";
    }
};

TEST_CASE("stringable")
{
    Simple::IStringable simple = make<Implementation>();
    REQUIRE(simple.ToString() == L"Simple");
}
