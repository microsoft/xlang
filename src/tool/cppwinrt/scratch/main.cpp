#include "pch.h"

using namespace std::literals;
using namespace winrt;
using namespace Windows::Foundation;

namespace winrt
{
    template <typename T, typename... Rest>
    T when_any2(T const& first, Rest const&... rest)
    {
        // TODO: static_assert that T and Rest are all the same type
        static_assert(impl::has_category_v<T>, "T must be WinRT async type such as IAsyncAction or IAsyncOperation<T>.");

        handle event{ check_pointer(WINRT_CreateEventW(nullptr, true, false, nullptr)) };
        auto lifetime = co_await impl::get_return_object();
        T result;

        auto completed = [&](auto&& async)
        {
            async.Completed([&, lifetime](auto sender, Windows::Foundation::AsyncStatus) noexcept
            {
                if (nullptr == _InterlockedCompareExchangePointer((void**)&result, get_abi(sender), nullptr))
                {
                    detach_abi(sender);
                    WINRT_VERIFY(WINRT_SetEvent(event.get()));
                }
            });
        };

        completed(first);
        (completed(rest), ...);
        co_await resume_on_signal(event.get());
        co_return result.GetResults();
    }
}



IAsyncOperation<TimeSpan> wait_for(TimeSpan timeout)
{
    co_await timeout;
    co_return timeout;
}

//IAsyncAction test(IAsyncAction& result)
//{
//    result = co_await impl::get_return_object();
//    co_return;
//}

int main()
{
    init_apartment();

    TimeSpan result = when_any2(wait_for(1s), wait_for(2s)).get();
    WINRT_ASSERT(result == 1s);

    getchar();
}
