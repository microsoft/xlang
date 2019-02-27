#pragma once
#include "Nested/MoreDerived.g.h"
#include "Nested.Derived.h"

namespace winrt::test_component_derived::Nested::implementation
{
    struct MoreDerived : MoreDerivedT<MoreDerived, test_component_derived::Nested::implementation::Derived>
    {
        MoreDerived() = default;

        MoreDerived(hstring const& name);
        void MoreDerivedMethod();
    };
}
namespace winrt::test_component_derived::Nested::factory_implementation
{
    struct MoreDerived : MoreDerivedT<MoreDerived, implementation::MoreDerived>
    {
    };
}
