#pragma once
#include "Nested/Derived.g.h"

namespace winrt::test_component_derived::Nested::implementation
{
    struct Derived : DerivedT<Derived>
    {
        Derived() = default;

        Derived(hstring const& name);
        void DerivedMethod();
    };
}
namespace winrt::test_component_derived::Nested::factory_implementation
{
    struct Derived : DerivedT<Derived, implementation::Derived>
    {
    };
}
