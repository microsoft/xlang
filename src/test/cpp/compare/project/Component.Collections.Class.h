#pragma once
#include "Component.Collections.Class.g.h"

namespace winrt::Component::Collections::implementation
{
    struct Class : ClassT<Class>
    {
        Class() = default;

        static Windows::Foundation::Collections::IIterable<hstring> Iterable();
        static Windows::Foundation::Collections::IVectorView<hstring> VectorView();
        static Windows::Foundation::Collections::IVector<hstring> Vector();
        static Windows::Foundation::Collections::IIterable<Windows::Foundation::Collections::IKeyValuePair<hstring, int32_t>> IterablePair();
        static Windows::Foundation::Collections::IMapView<hstring, int32_t> MapView();
        static Windows::Foundation::Collections::IMap<hstring, int32_t> Map();
        Windows::Foundation::Collections::IIterator<Windows::Foundation::Collections::IKeyValuePair<hstring, int32_t>> First();
        int32_t Lookup(hstring const&);
        uint32_t Size();
        bool HasKey(hstring const&);
        Windows::Foundation::Collections::IMapView<hstring, int32_t> GetView();
        bool Insert(hstring const&, int32_t const&);
        void Remove(hstring const&);
        void Clear();
        winrt::event_token MapChanged(Windows::Foundation::Collections::MapChangedEventHandler<hstring, int32_t> const&);
        void MapChanged(winrt::event_token const&) noexcept;
    };
}
namespace winrt::Component::Collections::factory_implementation
{
    struct Class : ClassT<Class, implementation::Class>
    {
    };
}
