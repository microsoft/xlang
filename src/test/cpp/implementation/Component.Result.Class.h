#pragma once
#include "Component.Result.Class.g.h"

namespace winrt::Component::Result::implementation
{
    struct Class
    {
        Class() = default;

        static int32_t GetInt32()
        {
            return 123;
        }

        static hstring GetString()
        {
            return L"Result";
        }

        static Component::Fast::FastClass GetFastClass()
        {
            return Component::Fast::FastClass();
        }

        static Component::Fast::SlowClass GetSlowClass()
        {
            return Component::Fast::SlowClass();
        }

        static Component::INotExclusive GetInterface()
        {
            return Component::Fast::SlowClass();
        }
    };
}
namespace winrt::Component::Result::factory_implementation
{
    struct Class : ClassT<Class, implementation::Class>
    {
    };
}
