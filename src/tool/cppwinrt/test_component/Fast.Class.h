#pragma once
#include "Fast.Class.g.h"

namespace winrt::test_component::Fast::implementation
{
    struct Class : ClassT<Class>
    {
        Class() = default;

        void Class_1();
        void Class_2();
        void Class_3();
    };
}
namespace winrt::test_component::Fast::factory_implementation
{
    struct Class : ClassT<Class, implementation::Class>
    {
    };
}
