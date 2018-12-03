#include "pch.h"
#include "Component.Params.Class.h"

namespace winrt::Component::Params::implementation
{
    void Class::SyncCall(hstring const&, Windows::Foundation::IReference<int32_t> const&, Windows::Foundation::Collections::IIterable<int32_t> const&, Windows::Foundation::Collections::IVectorView<int32_t> const&, Windows::Foundation::Collections::IMapView<int32_t, int32_t> const&, Windows::Foundation::Collections::IVector<int32_t> const&, Windows::Foundation::Collections::IMap<int32_t, int32_t> const&)
    {
        throw hresult_not_implemented();
    }
    Windows::Foundation::IAsyncAction Class::NotSyncCall(hstring const&, Windows::Foundation::IReference<int32_t> const&, Windows::Foundation::Collections::IIterable<int32_t> const&, Windows::Foundation::Collections::IVectorView<int32_t> const&, Windows::Foundation::Collections::IMapView<int32_t, int32_t> const&, Windows::Foundation::Collections::IVector<int32_t> const&, Windows::Foundation::Collections::IMap<int32_t, int32_t> const&)
    {
        throw hresult_not_implemented();
    }
}
