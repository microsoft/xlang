#include "pch.h"
#include "Composition.Visual.h"
#include "Composition.Visual.g.cpp"

namespace winrt::test_component::Composition::implementation
{
    void Visual::Offset()
    {
    }

    void Visual::ParentForTransform(test_component::Composition::Visual const& value)
    {
        value.Offset();
    }
}
