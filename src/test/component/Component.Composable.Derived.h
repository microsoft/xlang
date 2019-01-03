#pragma once
#include "Component.Composable.Derived.g.h"
#include "Component.Composable.Base.h"

namespace winrt::Component::Composable::implementation
{
    struct Derived : DerivedT<Derived, Component::Composable::implementation::Base>
    {
        Derived() = default;

        hstring DerivedMethod();
    };
}
namespace winrt::Component::Composable::factory_implementation
{
    struct Derived : DerivedT<Derived, implementation::Derived>
    {
    };
}
