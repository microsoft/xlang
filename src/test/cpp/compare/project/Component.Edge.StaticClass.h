#pragma once
#include "Component.Edge.StaticClass.g.h"

namespace winrt::Component::Edge::implementation
{
    struct StaticClass
    {
        StaticClass() = default;

        static void StaticMethod();
    };
}
namespace winrt::Component::Edge::factory_implementation
{
    struct StaticClass : StaticClassT<StaticClass, implementation::StaticClass>
    {
    };
}
