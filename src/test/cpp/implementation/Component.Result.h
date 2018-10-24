#pragma once
#include "Component.Result.g.h"

namespace winrt::Component::implementation
{
    struct Result : ResultT<Result>
    {
        Result() = default;

        int32_t GetInt32()
        {
            return 123;
        }

        hstring GetString()
        {
            return L"Result";
        }

        Component::Fast::FastClass GetFastClass()
        {
            return Component::Fast::FastClass();
        }

        Component::Fast::SlowClass GetSlowClass()
        {
            return Component::Fast::SlowClass();
        }

        Component::INotExclusive GetInterface()
        {
            return Component::Fast::FastClass();
        }
    };
}
namespace winrt::Component::factory_implementation
{
    struct Result : ResultT<Result, implementation::Result>
    {
    };
}
