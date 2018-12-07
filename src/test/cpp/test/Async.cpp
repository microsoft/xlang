#include "pch.h"
#include "winrt/Component.Async.h"
#include "winrt/Windows.Foundation.h"

using namespace winrt;
using namespace Component;
using namespace Windows::Foundation;

TEST_CASE("Async")
{
    IAsyncAction a = Async::Class::Action();
    a.get();

    IAsyncActionWithProgress<int32_t> b = Async::Class::ActionWithProgress();
    b.get();

    IAsyncOperation<hstring> c = Async::Class::Operation();
    REQUIRE(c.get() == L"Operation");

    IAsyncOperationWithProgress<hstring, int32_t> d = Async::Class::OperationWithProgress();
    REQUIRE(d.get() == L"OperationWithProgress");
}
