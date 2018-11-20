#include "pch.h"
#include "Component.Events.Class.h"

namespace winrt::Component::Events::implementation
{
    winrt::event_token Class::Static(Component::Events::Handler const&)
    {
        throw hresult_not_implemented();
    }
    void Class::Static(winrt::event_token const&) noexcept
    {
        throw hresult_not_implemented();
    }
    winrt::event_token Class::Member(Component::Events::Handler const&)
    {
        throw hresult_not_implemented();
    }
    void Class::Member(winrt::event_token const&) noexcept
    {
        throw hresult_not_implemented();
    }
}
