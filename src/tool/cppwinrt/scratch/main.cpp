#include "pch.h"

namespace winrt
{
    template <typename First, typename... Rest>
    First when_any2(First const& first, Rest const&... async)
    {
        // TODO: static_assert that First and Rest are all the same type
        static_assert(impl::has_category_v<First>, "Async must be WinRT async type such as IAsyncAction or IAsyncOperation<T>.");

        handle event{ check_pointer(WINRT_CreateEventW(nullptr, true, false, nullptr)) };
        First winner;

        auto completed = [&](auto&& async)
        {
            async.Completed([&, strong = co_await impl::get_return_object()](auto sender, Windows::Foundation::AsyncStatus) noexcept
            {
                if (nullptr == _InterlockedCompareExchangePointer((void**)&winner, get_abi(sender), nullptr))
                {
                    detach_abi(sender);
                    WINRT_VERIFY(WINRT_SetEvent(event.get()));
                }
            });
        };

        (completed(async), ...);
        co_await resume_on_signal(event.get());
        co_return winner.GetResults();
    }
}

using namespace std::literals;
using namespace winrt;
using namespace Windows::Foundation;

IAsyncOperation<TimeSpan> wait_for(TimeSpan timeout)
{
    co_await timeout;
    co_return timeout;
}

int main()
{
    TimeSpan result = winrt::when_any2(wait_for(1s), wait_for(2s)).get();
    //WINRT_ASSERT(result == 1s);
    getchar();
}
