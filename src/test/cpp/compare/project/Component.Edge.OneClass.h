#pragma once
#include "Component.Edge.OneClass.g.h"

namespace winrt::Component::Edge::implementation
{
    struct OneClass : OneClassT<OneClass>
    {
        OneClass() = default;

        OneClass(int32_t);
        static void StaticMethod(int32_t);
        void Method(int32_t);
        int32_t First();
    };
}
namespace winrt::Component::Edge::factory_implementation
{
    struct OneClass : OneClassT<OneClass, implementation::OneClass>
    {
    };
}
