#pragma once
#include "Component.Async.Class.g.h"

namespace winrt::Component::Async::implementation
{
    struct Class
    {
        Class() = default;

        static Windows::Foundation::IAsyncAction Action();
        static Windows::Foundation::IAsyncActionWithProgress<int32_t> ActionWithProgress();
        static Windows::Foundation::IAsyncOperation<hstring> Operation();
        static Windows::Foundation::IAsyncOperationWithProgress<hstring, int32_t> OperationWithProgress();
    };
}
namespace winrt::Component::Async::factory_implementation
{
    struct Class : ClassT<Class, implementation::Class>
    {
    };
}
