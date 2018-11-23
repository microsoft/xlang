#pragma once
#include "Component.Fast.FastClass.g.h"

namespace winrt::Component::Fast::implementation
{
    struct FastClass : FastClassT<FastClass>
    {
        FastClass() = default;

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
    struct FastClass : FastClassT<FastClass, implementation::FastClass>
    {
    };
}
