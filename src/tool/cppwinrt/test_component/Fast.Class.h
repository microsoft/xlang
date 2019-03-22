#pragma once
#include "Fast.Class.g.h"

namespace winrt::test_component::Fast::implementation
{
    struct Class : ClassT<Class>
    {
        Class() = default;

        hstring Class_1();
        hstring Class_2();
        hstring Class_3();
    };
}
namespace winrt::test_component::Fast::factory_implementation
{
    struct Class : ClassT<Class, implementation::Class>
    {
    };
}
