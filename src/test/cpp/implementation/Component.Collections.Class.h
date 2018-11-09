#pragma once
#include "Component.Collections.Class.g.h"

namespace winrt::Component::Collections::implementation
{
    struct Class
    {
        Class() = default;

        static Windows::Foundation::Collections::IIterable<hstring> Iterable()
        {
            return single_threaded_vector<hstring>({ L"Iterable" });
        }

        static Windows::Foundation::Collections::IVectorView<hstring> VectorView()
        {
            return single_threaded_vector<hstring>({ L"VectorView" }).GetView();
        }

        static Windows::Foundation::Collections::IVector<hstring> Vector()
        {
            return single_threaded_vector<hstring>({ L"Vector" });
        }

        static Windows::Foundation::Collections::IIterable<Windows::Foundation::Collections::IKeyValuePair<hstring, int32_t>> IterablePair()
        {
            return single_threaded_map<hstring, int32_t>(std::map<hstring, int32_t>{{ L"IterablePair", 1 }});
        }

        static Windows::Foundation::Collections::IMapView<hstring, int32_t> MapView()
        {
            return single_threaded_map<hstring, int32_t>(std::map<hstring, int32_t>{{ L"MapView", 1 }}).GetView();
        }

        static Windows::Foundation::Collections::IMap<hstring, int32_t> Map()
        {
            return single_threaded_map<hstring, int32_t>(std::map<hstring, int32_t>{{ L"Map", 1 }});
        }
    };
}
namespace winrt::Component::Collections::factory_implementation
{
    struct Class : ClassT<Class, implementation::Class>
    {
    };
}
