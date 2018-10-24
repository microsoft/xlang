#pragma once
#include "Component.Fast.SlowClass.g.h"

namespace winrt::Component::Fast::implementation
{
    struct SlowClass : SlowClassT<SlowClass>
    {
        SlowClass() = default;

        static hstring StaticMethod();
        hstring First();
        hstring Second();
        hstring Third();
        hstring Fourth();
        hstring NotExclusive();
    };
}
namespace winrt::Component::Fast::factory_implementation
{
    struct SlowClass : SlowClassT<SlowClass, implementation::SlowClass>
    {
    };
}
