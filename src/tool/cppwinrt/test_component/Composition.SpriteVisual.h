#pragma once
#include "Composition.SpriteVisual.g.h"
#include "Composition.Visual.h"

namespace winrt::test_component::Composition::implementation
{
    struct SpriteVisual : SpriteVisualT<SpriteVisual, test_component::Composition::implementation::Visual>
    {
        SpriteVisual() = default;

        void Brush();
        void Shadow();
    };
}
namespace winrt::test_component::Composition::factory_implementation
{
    struct SpriteVisual : SpriteVisualT<SpriteVisual, implementation::SpriteVisual>
    {
    };
}
