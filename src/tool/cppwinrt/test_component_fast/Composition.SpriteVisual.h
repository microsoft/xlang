#pragma once
#include "Composition.SpriteVisual.g.h"
#include "Composition.Visual.h"

namespace winrt::test_component_fast::Composition::implementation
{
    struct SpriteVisual : SpriteVisualT<SpriteVisual, implementation::Visual>
    {
        SpriteVisual() = default;

        void Brush();
        void Shadow();

        auto base_Visual()
        {
            return get_abi(static_cast<Composition::Visual const&>(*this));
        }
    };
}
