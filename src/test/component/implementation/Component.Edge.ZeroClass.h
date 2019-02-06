#pragma once
#include "Component.Edge.ZeroClass.g.h"

namespace winrt::Component::Edge::implementation
{
    struct ZeroClass : ZeroClassT<ZeroClass>
    {
        ZeroClass() = default;

        void Method()
        {
        }

        static void StaticMethod()
        {
        }
    };
}
namespace winrt::Component::Edge::factory_implementation
{
    struct ZeroClass : ZeroClassT<ZeroClass, implementation::ZeroClass>
    {
    };
}
