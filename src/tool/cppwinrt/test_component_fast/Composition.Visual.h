#pragma once
#include "Composition.Visual.g.h"
#include "Composition.CompositionObject.h"

namespace winrt::test_component_fast::Composition::implementation
{
    struct Visual : VisualT<Visual, test_component_fast::Composition::implementation::CompositionObject>
    {
        Visual() = default;

        void Offset();
        void ParentForTransform(test_component_fast::Composition::Visual const& value);
    };
}
