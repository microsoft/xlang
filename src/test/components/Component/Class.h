#pragma once

#include "Class.g.h"

namespace winrt::Component::implementation
{
    struct Class : ClassT<Class>
    {
        Class() = default;

        hstring GetValue() const noexcept;
    };
}

namespace winrt::Component::factory_implementation
{
    struct Class : ClassT<Class, implementation::Class>
    {

    };
}