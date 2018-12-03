#pragma once
#include "Component.Params.Class.g.h"

namespace winrt::Component::Params::implementation
{
    struct Class : ClassT<Class>
    {
        Class() = default;

        void SyncCall(hstring const&, Windows::Foundation::IReference<int32_t> const&, Windows::Foundation::Collections::IIterable<int32_t> const&, Windows::Foundation::Collections::IVectorView<int32_t> const&, Windows::Foundation::Collections::IMapView<int32_t, int32_t> const&, Windows::Foundation::Collections::IVector<int32_t> const&, Windows::Foundation::Collections::IMap<int32_t, int32_t> const&);
        Windows::Foundation::IAsyncAction NotSyncCall(hstring const&, Windows::Foundation::IReference<int32_t> const&, Windows::Foundation::Collections::IIterable<int32_t> const&, Windows::Foundation::Collections::IVectorView<int32_t> const&, Windows::Foundation::Collections::IMapView<int32_t, int32_t> const&, Windows::Foundation::Collections::IVector<int32_t> const&, Windows::Foundation::Collections::IMap<int32_t, int32_t> const&);
        Component::Structs::Simple Structs(Component::Structs::Simple const&, Component::Structs::Simple&);
    };
}
namespace winrt::Component::Params::factory_implementation
{
    struct Class : ClassT<Class, implementation::Class>
    {
    };
}
