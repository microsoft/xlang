#pragma once
#include "Composition.CompositionObject.g.h"

namespace winrt::test_component::Composition::implementation
{
    struct CompositionObject : CompositionObjectT<CompositionObject>
    {
        CompositionObject() = default;

        void Close();
        void Compositor();
        void StartAnimationGroup();
    };
}
