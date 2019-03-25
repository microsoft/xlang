#include "pch.h"
#include "Composition.CompositionObject.h"
#include "Composition.CompositionObject.g.cpp"

namespace winrt::test_component_fast::Composition::implementation
{
    void CompositionObject::Close()
    {
        throw hresult_not_implemented();
    }
    void CompositionObject::Compositor()
    {
        throw hresult_not_implemented();
    }
    void CompositionObject::StartAnimationGroup()
    {
        throw hresult_not_implemented();
    }
}
