#pragma once
#include "Component.Edge.TwoClass.g.h"

namespace winrt::Component::Edge::implementation
{
    struct TwoClass : TwoClassT<TwoClass>
    {
        TwoClass() = default;

        static void StaticMethod(int32_t, int32_t);
        TwoClass(int32_t, int32_t);
        void Method(int32_t, int32_t);
        int32_t First();
        int32_t Second();
    };
}
namespace winrt::Component::Edge::factory_implementation
{
    struct TwoClass : TwoClassT<TwoClass, implementation::TwoClass>
    {
    };
}
