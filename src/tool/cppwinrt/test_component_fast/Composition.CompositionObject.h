#pragma once
#include "Composition.CompositionObject.g.h"

namespace winrt::test_component_fast::Composition::implementation
{
    struct CompositionObject : CompositionObjectT<CompositionObject>
    {
        CompositionObject() = default;

        void Close();
        void Compositor();
        void StartAnimationGroup();
    };
}
