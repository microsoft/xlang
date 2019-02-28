#pragma once
#include "Nested/HierarchyD.g.h"
#include "Nested.HierarchyC.h"

namespace winrt::test_component_derived::Nested::implementation
{
    struct HierarchyD : HierarchyDT<HierarchyD, test_component_derived::Nested::implementation::HierarchyC>
    {
        HierarchyD() = default;

        HierarchyD(hstring const& name);
        void HierarchyD_Method();
    };
}
namespace winrt::test_component_derived::Nested::factory_implementation
{
    struct HierarchyD : HierarchyDT<HierarchyD, implementation::HierarchyD>
    {
    };
}
