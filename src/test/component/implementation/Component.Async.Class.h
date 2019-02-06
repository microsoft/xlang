#pragma once
#include "Component.Async.Class.g.h"

namespace winrt::Component::Async::implementation
{
    struct Class
    {
        Class() = default;

        static Windows::Foundation::IAsyncAction Action()
        {
            co_return;
        }

        static Windows::Foundation::IAsyncActionWithProgress<int32_t> ActionWithProgress()
        {
            co_return;
        }

        static Windows::Foundation::IAsyncOperation<hstring> Operation()
        {
            co_return L"Operation";
        }

        static Windows::Foundation::IAsyncOperationWithProgress<hstring, int32_t> OperationWithProgress()
        {
            co_return L"OperationWithProgress";
        }
    };
}
namespace winrt::Component::Async::factory_implementation
{
    struct Class : ClassT<Class, implementation::Class>
    {
    };
}
