#include "pch.h"
#include "Component.Async.Class.h"

namespace winrt::Component::Async::implementation
{
    Windows::Foundation::IAsyncAction Class::Action();
    {
        throw hresult_not_implemented();
    }
    Windows::Foundation::IAsyncActionWithProgress<int32_t> Class::ActionWithProgress();
    {
        throw hresult_not_implemented();
    }
    Windows::Foundation::IAsyncOperation<hstring> Class::Operation();
    {
        throw hresult_not_implemented();
    }
    Windows::Foundation::IAsyncOperationWithProgress<hstring, int32_t> Class::OperationWithProgress();
    {
        throw hresult_not_implemented();
    }
}
