#include "catch.hpp"
#include <windows.h>
#include "winrt/Windows.Foundation.h"

using namespace winrt;
using namespace Windows::Foundation;

namespace
{
    IAsyncAction Action(HANDLE event)
    {
        co_await resume_on_signal(event);
    }

    IAsyncActionWithProgress<int> ActionWithProgress(HANDLE event)
    {
        co_await resume_on_signal(event);
    }

    IAsyncOperation<int> Operation(HANDLE event)
    {
        co_await resume_on_signal(event);
        co_return 1;
    }

    IAsyncOperationWithProgress<int, int> OperationWithProgress(HANDLE event)
    {
        co_await resume_on_signal(event);
        co_return 1;
    }

    IAsyncAction Await()
    {
        // Manual reset so that all waiters will resume and initially set so they won't block.
        handle event{ CreateEvent(nullptr, true, true, nullptr) };

        co_await Action(event.get());
        co_await ActionWithProgress(event.get());
        co_await Operation(event.get());
        co_await OperationWithProgress(event.get());
    }

    template <typename F>
    void CheckState(F make)
    {
        handle start{ CreateEvent(nullptr, true, false, nullptr) };
        handle completed{ CreateEvent(nullptr, true, false, nullptr) };
        auto async = make(start.get());
        REQUIRE(async.Status() == AsyncStatus::Started);

        async.Completed([&](auto&& sender, AsyncStatus status)
            {
                REQUIRE(async == sender);
                REQUIRE(status == AsyncStatus::Completed);
                SetEvent(completed.get());
            });

        // Still in Started state waiting for signal.
        Sleep(100);
        REQUIRE(WaitForSingleObject(completed.get(), 0) == WAIT_TIMEOUT);
        REQUIRE(async.Status() == AsyncStatus::Started);

        // Signal async to run.
        SetEvent(start.get());

        // Wait for async to complete.
        REQUIRE(WaitForSingleObject(completed.get(), 1000) == WAIT_OBJECT_0);

        REQUIRE(async.Status() == AsyncStatus::Completed);
        REQUIRE(async.ErrorCode() == S_OK);
    }
}

TEST_CASE("AsyncSuspend")
{
    handle start{ CreateEvent(nullptr, true, true, nullptr) };
    Action(start.get()).get();
    ActionWithProgress(start.get()).get();
    Operation(start.get()).get();
    OperationWithProgress(start.get()).get();
    Await().get();

    CheckState(Action);
    CheckState(ActionWithProgress);
    CheckState(Operation);
    CheckState(OperationWithProgress);
}
