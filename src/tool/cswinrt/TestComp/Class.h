#pragma once

#include "Class.g.h"

namespace winrt::TestComp::implementation
{
    struct Class : ClassT<Class>
    {
        Class() = default;

        int32_t _int = 0;
        winrt::event<Windows::Foundation::TypedEventHandler<TestComp::Class, Windows::Foundation::IInspectable>> _intChanged;
        winrt::hstring _string;
        winrt::hstring _string2;
        winrt::event<Windows::Foundation::TypedEventHandler<TestComp::Class, hstring>> _stringChanged;
        Windows::Foundation::Collections::IVector<hstring> _strings;

        int32_t IntProperty();
        void IntProperty(int32_t value);
        winrt::event_token IntPropertyChanged(Windows::Foundation::TypedEventHandler<TestComp::Class, Windows::Foundation::IInspectable> const& handler);
        void IntPropertyChanged(winrt::event_token const& token) noexcept;
        hstring StringProperty();
        void StringProperty(hstring const& value);
        winrt::event_token StringPropertyChanged(Windows::Foundation::TypedEventHandler<TestComp::Class, hstring> const& handler);
        void StringPropertyChanged(winrt::event_token const& token) noexcept;
        hstring StringProperty2();
        void StringProperty2(hstring const& value);
        Windows::Foundation::Collections::IVector<hstring> StringsProperty();
        void StringsProperty(Windows::Foundation::Collections::IVector<hstring> value);
        void GetString();
        void SetString(TestComp::ProvideString const& provider);
    };
}

namespace winrt::TestComp::factory_implementation
{
    struct Class : ClassT<Class, implementation::Class>
    {
    };
}
