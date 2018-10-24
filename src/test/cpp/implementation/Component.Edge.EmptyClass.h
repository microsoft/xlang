#pragma once
#include "Component.Edge.EmptyClass.g.h"

namespace winrt::Component::Edge::implementation
{
    struct EmptyClass : EmptyClassT<EmptyClass>
    {
        EmptyClass() = default;
    };
}
