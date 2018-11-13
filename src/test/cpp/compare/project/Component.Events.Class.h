#pragma once
#include "Component.Events.Class.g.h"

namespace winrt::Component::Events::implementation
{
    struct Class : ClassT<Class>
    {
        Class() = default;

        static winrt::event_token Static(Component::Events::Handler const&);
        static void Static(winrt::event_token const&) noexcept;
        winrt::event_token Member(Component::Events::Handler const&);
        void Member(winrt::event_token const&) noexcept;
    };
}
namespace winrt::Component::Events::factory_implementation
{
    struct Class : ClassT<Class, implementation::Class>
    {
    };
}
