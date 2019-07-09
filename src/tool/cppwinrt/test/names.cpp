#include "pch.h"

using namespace winrt;
using namespace Windows::Foundation;

void check_terminated(winrt::param::hstring const&)
{
}

TEST_CASE("names")
{
    REQUIRE(name_of<Windows::Foundation::IUnknown>() == L"{00000000-0000-0000-c000-000000000046}"sv);
    STATIC_REQUIRE(name_of<IInspectable>() == L"Object"sv);
    STATIC_REQUIRE(name_of<EventHandler<guid>>() == L"Windows.Foundation.EventHandler`1<Guid>"sv);
    STATIC_REQUIRE(name_of<TypedEventHandler<guid, Point>>() == L"Windows.Foundation.TypedEventHandler`2<Guid, Windows.Foundation.Point>"sv);

    check_terminated(name_of<Windows::Foundation::IUnknown>());
    check_terminated(name_of<IInspectable>());
    check_terminated(name_of<EventHandler<guid>>());
    check_terminated(name_of<TypedEventHandler<guid, Point>>());
}
