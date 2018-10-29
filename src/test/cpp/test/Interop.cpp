#include "catch.hpp"
#include <inspectable.h>
#include "winrt/base.h"

using namespace winrt;

Windows::Foundation::IAsyncAction Async()
{
    co_return;
}

TEST_CASE("Interop")
{
    Windows::Foundation::IAsyncAction a = Async();
    com_ptr<::IInspectable> b = a.as<::IInspectable>();
    Windows::Foundation::IAsyncAction c = b.as<Windows::Foundation::IAsyncAction>();
    REQUIRE(a == c);
}
