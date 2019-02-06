#pragma once
#include "Component.Events.Class.g.h"

namespace winrt::Component::Events::implementation
{
    struct Class : ClassT<Class>
    {
        Class() = default;

        winrt::event_token Member(Component::Events::Handler const& handler)
        {
            return m_member.add(handler);
        }

        void Member(winrt::event_token const& token)
        {
            m_member.remove(token);
        }

        event<Component::Events::Handler> m_member;
    };
}
namespace winrt::Component::Events::factory_implementation
{
    struct Class : ClassT<Class, implementation::Class, static_lifetime>
    {
        winrt::event_token Static(Component::Events::Handler const& handler)
        {
            return m_static.add(handler);
        }

        void Static(winrt::event_token const& token)
        {
            m_static.remove(token);
        }

        event<Component::Events::Handler> m_static;
    };
}
