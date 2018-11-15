#include "pch.h"
#include "Component.Collections.Class.h"
#include "Component.Collections.Class.g.cpp"

namespace winrt::Component::Collections::implementation
{
    Windows::Foundation::Collections::IIterator<Windows::Foundation::Collections::IKeyValuePair<hstring, int32_t>> Class::First()
    {
        throw hresult_not_implemented();
    }
    int32_t Class::Lookup(hstring const&)
    {
        throw hresult_not_implemented();
    }
    uint32_t Class::Size()
    {
        throw hresult_not_implemented();
    }
    bool Class::HasKey(hstring const&)
    {
        throw hresult_not_implemented();
    }
    Windows::Foundation::Collections::IMapView<hstring, int32_t> Class::GetView()
    {
        throw hresult_not_implemented();
    }
    bool Class::Insert(hstring const&, int32_t const&)
    {
        throw hresult_not_implemented();
    }
    void Class::Remove(hstring const&)
    {
        throw hresult_not_implemented();
    }
    void Class::Clear()
    {
        throw hresult_not_implemented();
    }
    winrt::event_token Class::MapChanged(Windows::Foundation::Collections::MapChangedEventHandler<hstring, int32_t> const&)
    {
        throw hresult_not_implemented();
    }
    void Class::MapChanged(winrt::event_token const&)
    {
        throw hresult_not_implemented();
    }
}
