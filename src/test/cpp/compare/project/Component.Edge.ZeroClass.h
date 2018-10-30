#pragma once
#include "Component.Edge.ZeroClass.g.h"

namespace winrt::Component::Edge::implementation
{
    struct ZeroClass : ZeroClassT<ZeroClass>
    {
        ZeroClass() = default;

        static void StaticMethod();
        void Method();
    };
}
namespace winrt::Component::Edge::factory_implementation
{
    struct ZeroClass : ZeroClassT<ZeroClass, implementation::ZeroClass>
    {
    };
}
