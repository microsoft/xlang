#include "pch.h"

using namespace std::literals;
using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::System;

IAsyncAction ForegroundAsync(DispatcherQueue dispatcher)
{
    co_await resume_foreground(dispatcher);
}

IAsyncAction BackgroundAsync(DispatcherQueue dispatcher)
{
    co_await resume_background();
    int counter{};

    while (true)
    {
        co_await ForegroundAsync(dispatcher);
        Sleep(1000);
        printf("%d\n", ++counter);
    }
}

int main()
{
    auto controller = DispatcherQueueController::CreateOnDedicatedThread();
    auto dispatcher = controller.DispatcherQueue();
    auto timer = dispatcher.CreateTimer();
    timer.Interval(100ms);
    timer.Tick([](auto&& ...) { printf("."); });
    timer.Start();

    BackgroundAsync(dispatcher).get();
}

////////////////////////////

#include "pch.h"

using namespace std::literals;
using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::System;

static handle signaled{ CreateEventW(nullptr, true, false, nullptr) };

IAsyncAction ForegroundAsync(DispatcherQueue dispatcher)
{
    co_await resume_foreground(dispatcher);
}

fire_and_forget SignalFromForeground(DispatcherQueue dispatcher)
{
    co_await resume_foreground(dispatcher);
    assert(SetEvent(signaled.get()));
}

IAsyncAction BackgroundAsync(DispatcherQueue dispatcher)
{
    co_await resume_background();
    uint32_t const id = GetCurrentThreadId();
    co_await ForegroundAsync(dispatcher);
    assert(id == GetCurrentThreadId());
    SignalFromForeground(dispatcher);
    assert(WAIT_OBJECT_0 == WaitForSingleObject(signaled.get(), INFINITE));
}

int main()
{
    auto controller = DispatcherQueueController::CreateOnDedicatedThread();
    auto dispatcher = controller.DispatcherQueue();

    BackgroundAsync(dispatcher).get();
}


