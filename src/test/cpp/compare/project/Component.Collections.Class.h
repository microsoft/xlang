#pragma once
#include "Component.Collections.Class.g.h"

namespace winrt::Component::Collections::implementation
{
    struct Class
    {
        Class() = default;

        static Windows::Foundation::Collections::IIterable<hstring> Iterable();
        static Windows::Foundation::Collections::IVectorView<hstring> VectorView();
        static Windows::Foundation::Collections::IVector<hstring> Vector();
        static Windows::Foundation::Collections::IIterable<Windows::Foundation::Collections::IKeyValuePair<hstring, int32_t>> IterablePair();
        static Windows::Foundation::Collections::IMapView<hstring, int32_t> MapView();
        static Windows::Foundation::Collections::IMap<hstring, int32_t> Map();
    };
}
namespace winrt::Component::Collections::factory_implementation
{
    struct Class : ClassT<Class, implementation::Class>
    {
    };
}
