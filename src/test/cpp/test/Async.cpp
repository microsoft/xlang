#include "pch.h"
#include "winrt/Component.Async.h"

using namespace winrt;
using namespace Component;
using namespace Windows::Foundation;

TEST_CASE("Async")
{
    IAsyncAction a = Async::Class::Action();
    a.get();

    IAsyncActionWithProgress<int32> b = Async::Class::ActionWithProgress();
    b.get();

    IAsyncOperation<hstring> c = Async::Class::Operation();
    REQUIRE(c.get() == L"Operation");

    IAsyncOperationWithProgress<hstring, int32> c = Async::Class::OperationWithProgress();
    REQUIRE(c.get() == L"OperationWithProgress");
}
