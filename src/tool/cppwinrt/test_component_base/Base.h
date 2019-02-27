#pragma once
#include "Base.g.h"

namespace winrt::test_component_base::implementation
{
    struct Base : BaseT<Base>
    {
        Base() = default;

        Base(hstring const& name);
        void BaseMethod();
    };
}
namespace winrt::test_component_base::factory_implementation
{
    struct Base : BaseT<Base, implementation::Base>
    {
    };
}
