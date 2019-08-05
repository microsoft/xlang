#include "pch.h"

using namespace std::literals;
using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::System;

//#if 0
//
//IAsyncAction ForegroundAsync(DispatcherQueue dispatcher)
//{
//    assert(!impl::is_sta());
//    co_await resume_foreground(dispatcher);
//    printf("ForegroundAsync %d\n", GetCurrentThreadId());
//    assert(impl::is_sta());
//}
//
//IAsyncAction BackgroundAsync(DispatcherQueue dispatcher)
//{
//    co_await resume_background();
//    printf("BackgroundAsync %d\n", GetCurrentThreadId());
//    int counter{};
//
//    while (true)
//    {
//        co_await ForegroundAsync(dispatcher);
//        Sleep(1000);
//        printf("%d\n", ++counter);
//    }
//}
//
//int main()
//{
//    auto controller = DispatcherQueueController::CreateOnDedicatedThread();
//    auto dispatcher = controller.DispatcherQueue();
//    auto timer = dispatcher.CreateTimer();
//    timer.Interval(100ms);
//    timer.Tick([](auto&& ...) { printf("."); });
//    timer.Start();
//
//    BackgroundAsync(dispatcher).get();
//}
//
//#else
//

int main()
{
}



/////////////////////////////////

//IAsyncAction ForegroundAsync(DispatcherQueue dispatcher)
//{
//    assert(!impl::is_sta());
//    co_await resume_foreground(dispatcher);
//    printf("ForegroundAsync %d\n", GetCurrentThreadId());
//    assert(impl::is_sta());
//    Sleep(100);
//}
//
//IAsyncAction BackgroundAsync(DispatcherQueue dispatcher)
//{
//    co_await resume_background();
//    assert(!impl::is_sta());
//    printf("BackgroundAsync %d\n", GetCurrentThreadId());
//    co_await ForegroundAsync(dispatcher);
//    assert(!impl::is_sta());
//    co_await ForegroundAsync(dispatcher);
//    assert(!impl::is_sta());
//}
//
//int main()
//{
//    printf("main %d\n", GetCurrentThreadId());
//    auto controller = DispatcherQueueController::CreateOnDedicatedThread();
//    auto dispatcher = controller.DispatcherQueue();
//
//    BackgroundAsync(dispatcher).get();
//    getchar();
//}
