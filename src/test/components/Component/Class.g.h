#pragma once

#include <winrt/Component.h>

namespace winrt::Component::implementation
{
    template <typename D, typename ... I>
    struct WINRT_EBO Class_base : implements<D, Component::IClass, I...>
    {
        using base_type = Class_base;
        using class_type = Component::Class;
        using implements_type = typename Class_base::implements_type;
        using implements_type::implements_type;

        operator impl::producer_ref<class_type> const() const noexcept
        {
            return { to_abi<default_interface<class_type>>(this) };
        }

        hstring GetRuntimeClassName() const
        {
            return L"Component.Class";
        }
    };
}

namespace winrt::Component::factory_implementation
{
    template <typename D, typename T, typename ... I>
    struct WINRT_EBO ClassT : implements<D, Windows::Foundation::IActivationFactory, I...>
    {
        hstring GetRuntimeClassName() const
        {
            return L"Component.Class";
        }

        Windows::Foundation::IInspectable ActivateInstance() const
        {
            return make<T>();
        }
    };
}

namespace winrt::Component::implementation
{
    template <typename D, typename ... I>
    using ClassT = Class_base<D, I...>;
}