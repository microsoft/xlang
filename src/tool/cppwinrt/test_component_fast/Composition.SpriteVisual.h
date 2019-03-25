#pragma once
#include "Composition.SpriteVisual.g.h"
#include "Composition.Visual.h"

namespace winrt::test_component_fast::Composition::implementation
{
    struct SpriteVisual : SpriteVisualT<SpriteVisual, test_component_fast::Composition::implementation::Visual>
    {
        SpriteVisual() = default;

        void Brush();
        void Shadow();
    };
}
namespace winrt::test_component_fast::Composition::factory_implementation
{
    struct SpriteVisual : SpriteVisualT<SpriteVisual, implementation::SpriteVisual>
    {
    };
}
