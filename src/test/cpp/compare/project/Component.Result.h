#pragma once
#include "Component.Result.g.h"

namespace winrt::Component::implementation
{
    struct Result : ResultT<Result>
    {
        Result() = default;

        int32_t GetInt32();
        hstring GetString();
        Component::Fast::FastClass GetFastClass();
        Component::Fast::SlowClass GetSlowClass();
        Component::INotExclusive GetInterface();
    };
}
namespace winrt::Component::factory_implementation
{
    struct Result : ResultT<Result, implementation::Result>
    {
    };
}
