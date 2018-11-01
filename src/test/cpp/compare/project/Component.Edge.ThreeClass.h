#pragma once
#include "Component.Edge.ThreeClass.g.h"

namespace winrt::Component::Edge::implementation
{
    struct ThreeClass : ThreeClassT<ThreeClass>
    {
        ThreeClass() = default;

        ThreeClass(int32_t, int32_t, int32_t);
        static void StaticMethod(int32_t, int32_t, int32_t);
        void Method(int32_t, int32_t, int32_t);
        int32_t First();
        int32_t Second();
        int32_t Third();
    };
}
namespace winrt::Component::Edge::factory_implementation
{
    struct ThreeClass : ThreeClassT<ThreeClass, implementation::ThreeClass>
    {
    };
}
