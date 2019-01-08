#include "catch.hpp"
#include <windows.h>
#include "winrt/Windows.Foundation.h"

using namespace winrt;
using namespace Windows::Foundation;
using namespace std::chrono_literals;

namespace
{
    IAsyncAction Action()
    {
        co_await 10ms;
        throw hresult_invalid_argument(L"Async");
    }

    IAsyncActionWithProgress<int> ActionWithProgress()
    {
        co_await 10ms;
        throw hresult_invalid_argument(L"Async");
    }

    IAsyncOperation<int> Operation()
    {
        co_await 10ms;
        throw hresult_invalid_argument(L"Async");
        co_return 1;
    }

    IAsyncOperationWithProgress<int, int> OperationWithProgress()
    {
        co_await 10ms;
        throw hresult_invalid_argument(L"Async");
        co_return 1;
    }

    template <typename F>
    void Check(F make)
    {
        try
        {
            make().get();
            REQUIRE(false);
        }
        catch (hresult_invalid_argument const& e)
        {
            REQUIRE(e.message() == L"Async");
        }
    }
}

TEST_CASE("AsyncThrow")
{
    Check(Action);
    Check(ActionWithProgress);
    Check(Operation);
    Check(OperationWithProgress);
}
