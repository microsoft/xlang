#include "pch.h"
#include "Class.h"
#include "Class.g.cpp"

#include "hstring.h"

static constexpr auto x = sizeof(HSTRING_HEADER);

namespace winrt::TestComp::implementation
{
    int32_t Class::IntProperty()
    {
        return _int;
    }
    void Class::IntProperty(int32_t value)
    {
        _int = value;
        _intChanged(*this, winrt::box_value<int32_t>(_int));
    }
    winrt::event_token Class::IntPropertyChanged(Windows::Foundation::TypedEventHandler<TestComp::Class, Windows::Foundation::IInspectable> const& handler)
    {
        return _intChanged.add(handler);
    }
    void Class::IntPropertyChanged(winrt::event_token const& token) noexcept
    {
        _intChanged.remove(token);
    }
    hstring Class::StringProperty()
    {
        return _string;
    }
    void Class::StringProperty(hstring const& value)
    {
        _string = value;
    }
    winrt::event_token Class::StringPropertyChanged(Windows::Foundation::TypedEventHandler<TestComp::Class, hstring> const& handler)
    {
        return _stringChanged.add(handler);
    }
    void Class::StringPropertyChanged(winrt::event_token const& token) noexcept
    {
        _stringChanged.remove(token);
    }
    hstring Class::StringProperty2()
    {
        return _string2;
    }
    void Class::StringProperty2(hstring const& value)
    {
        _string2 = value;
    }
    Windows::Foundation::Collections::IVector<hstring> Class::StringsProperty()
    {
        return _strings;
    }
    void Class::StringsProperty(Windows::Foundation::Collections::IVector<hstring> value)
    {
        _strings = value;
    }
    void Class::GetString()
    {
        _stringChanged(*this, _string);
    }
    void Class::SetString(TestComp::ProvideString const& provider)
    {
        _string = provider();
    }
}
