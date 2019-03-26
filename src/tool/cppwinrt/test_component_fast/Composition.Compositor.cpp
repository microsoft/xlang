#include "pch.h"
#include "Composition.Compositor.h"
#include "Composition.Compositor.g.cpp"

namespace winrt::test_component_fast::Composition::implementation
{
    SpriteVisual Compositor::CreateSpriteVisual()
    {
        return SpriteVisual();
    }
}
