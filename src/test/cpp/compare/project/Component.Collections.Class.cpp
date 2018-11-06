#include "pch.h"
#include "Component.Collections.Class.h"

namespace winrt::Component::Collections::implementation
{
    Windows::Foundation::Collections::IIterable<hstring> Class::Iterable();
    {
        throw hresult_not_implemented();
    }
    Windows::Foundation::Collections::IVectorView<hstring> Class::VectorView();
    {
        throw hresult_not_implemented();
    }
    Windows::Foundation::Collections::IVector<hstring> Class::Vector();
    {
        throw hresult_not_implemented();
    }
    Windows::Foundation::Collections::IIterable<Windows::Foundation::Collections::IKeyValuePair<hstring, int32_t>> Class::IterablePair();
    {
        throw hresult_not_implemented();
    }
    Windows::Foundation::Collections::IMapView<hstring, int32_t> Class::MapView();
    {
        throw hresult_not_implemented();
    }
    Windows::Foundation::Collections::IMap<hstring, int32_t> Class::Map();
    {
        throw hresult_not_implemented();
    }
}
