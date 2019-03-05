#pragma once
#include "Composition.Visual.g.h"
#include "Composition.CompositionObject.h"

namespace winrt::test_component::Composition::implementation
{
    struct Visual : VisualT<Visual, test_component::Composition::implementation::CompositionObject>
    {
        Visual() = default;

        void Offset();
        void ParentForTransform(test_component::Composition::Visual const& value);
    };
}
