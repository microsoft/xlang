#pragma once
#include "Component.Result.Class.g.h"

namespace winrt::Component::Result::implementation
{
    struct Class
    {
        Class() = default;

        static int32_t GetInt32();
        static hstring GetString();
        static Component::Fast::FastClass GetFastClass();
        static Component::Fast::SlowClass GetSlowClass();
        static Component::INotExclusive GetInterface();
    };
}
namespace winrt::Component::Result::factory_implementation
{
    struct Class : ClassT<Class, implementation::Class>
    {
    };
}
