#pragma once
#include "Component.Composable.Base.g.h"

namespace winrt::Component::Composable::implementation
{
    struct Base : BaseT<Base>
    {
        Base() = default;

        hstring BaseMethod();
    };
}
namespace winrt::Component::Composable::factory_implementation
{
    struct Base : BaseT<Base, implementation::Base>
    {
    };
}
