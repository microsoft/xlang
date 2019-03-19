#pragma once
#include "Velocity.Class1.g.h"

namespace winrt::test_component::Velocity::implementation
{
    struct Class1 : Class1T<Class1>
    {
        Class1() = default;

    };
}
namespace winrt::test_component::Velocity::factory_implementation
{
    struct Class1 : Class1T<Class1, implementation::Class1>
    {
    };
}
