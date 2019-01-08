#include "catch.hpp"
#include <windows.h>
#include "winrt/Windows.Foundation.h"

using namespace winrt;
using namespace Windows::Foundation;

namespace
{
    //
    // Checks that the cancellation callback is invoked.
    //

    IAsyncAction Action(HANDLE event, bool& canceled)
    {
        auto cancel = co_await get_cancellation_token();

        cancel.callback([&]
            {
                REQUIRE(!canceled);
                canceled = true;
            });

        co_await resume_on_signal(event);
        co_await std::experimental::suspend_never();
        REQUIRE(false);
    }

    IAsyncActionWithProgress<int> ActionWithProgress(HANDLE event, bool& canceled)
    {
        auto cancel = co_await get_cancellation_token();

        cancel.callback([&]
            {
                REQUIRE(!canceled);
                canceled = true;
            });

        co_await resume_on_signal(event);
        co_await std::experimental::suspend_never();
        REQUIRE(false);
    }

    IAsyncOperation<int> Operation(HANDLE event, bool& canceled)
    {
        auto cancel = co_await get_cancellation_token();

        cancel.callback([&]
            {
                REQUIRE(!canceled);
                canceled = true;
            });

        co_await resume_on_signal(event);
        co_await std::experimental::suspend_never();
        REQUIRE(false);
        co_return 1;
    }

    IAsyncOperationWithProgress<int, int> OperationWithProgress(HANDLE event, bool& canceled)
    {
        auto cancel = co_await get_cancellation_token();

        cancel.callback([&]
            {
                REQUIRE(!canceled);
                canceled = true;
            });

        co_await resume_on_signal(event);
        co_await std::experimental::suspend_never();
        REQUIRE(false);
        co_return 1;
    }

    template <typename F>
    void Check(F make)
    {
        handle event{ CreateEvent(nullptr, true, false, nullptr) };
        bool canceled = false;
        auto async = make(event.get(), canceled);
        async.Cancel();
        REQUIRE(canceled);
        SetEvent(event.get());
    }
}

TEST_CASE("AsyncCancelCallback")
{
    Check(Action);
    Check(ActionWithProgress);
    Check(Operation);
    Check(OperationWithProgress);
}
