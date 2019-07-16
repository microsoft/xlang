#include "pch.h"
#include <pplawait.h>

using namespace concurrency;
using namespace winrt;
using namespace Windows::Foundation;

task<void> ppl(bool& done)
{
    co_await resume_background();
    done = true;
}

IAsyncAction async(bool& done)
{
    co_await resume_background();
    done = true;
}

IAsyncAction when_signaled(handle const& event)
{
    co_await resume_on_signal(event.get());
}

IAsyncAction done()
{
    co_return;
}

IAsyncOperation<int> done_int()
{
    co_return 0;
}

TEST_CASE("when")
{
    {
        bool ppl_done = false;
        bool async_done = false;

        // Ensures that different async types can be aggregated.
        winrt::when_all(ppl(ppl_done), async(async_done)).get();

        REQUIRE(ppl_done);
        REQUIRE(async_done);
    }
    {
        // Ensures that different WinRT async types can be aggregated.
        winrt::when_any(done(), done_int()).get();

        handle first_event{ check_pointer(CreateEventW(nullptr, true, false, nullptr)) };
        handle second_event{ check_pointer(CreateEventW(nullptr, true, false, nullptr)) };

        IAsyncAction any = winrt::when_any(when_signaled(first_event), when_signaled(second_event));

        // Make sure we're still waiting.
        Sleep(100);
        REQUIRE(any.Status() == AsyncStatus::Started);

        // Allow only one of the async objects to complete.
        SetEvent(second_event.get());

        // This should now complete.
        any.get();

        SetEvent(first_event.get());
    }
}
