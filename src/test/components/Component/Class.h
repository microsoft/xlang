#pragma once

#include <winrt/base.h>

namespace winrt::Component::factory_implementation
{
    struct Class : implements<Class, Windows::Foundation::IActivationFactory>
    {
        Windows::Foundation::IInspectable ActivateInstance() const
        {
            throw hresult_not_implemented{};
        }
    };
}