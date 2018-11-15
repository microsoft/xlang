#include "pch.h"
#include "winrt/Component.Collections.h"

using namespace winrt;
using namespace Component;
using namespace Windows::Foundation::Collections;

TEST_CASE("Collections")
{
    IIterable<hstring> i = Collections::Class::Iterable();
    REQUIRE(i.First().Current() == L"Iterable");

    IVectorView<hstring> vv = Collections::Class::VectorView();
    REQUIRE(vv.GetAt(0) == L"VectorView");

    IVector<hstring> v = Collections::Class::Vector();
    REQUIRE(v.GetAt(0) == L"Vector");

    IIterable<IKeyValuePair<hstring, int32_t>> ip = Collections::Class::IterablePair();
    REQUIRE(ip.First().Current().Key() == L"IterablePair");

    IMapView<hstring, int32_t> mv = Collections::Class::MapView();
    REQUIRE(mv.Lookup(L"MapView") == 1);

    IMap<hstring, int32_t> m = Collections::Class::Map();
    REQUIRE(m.Lookup(L"Map") == 1);

    Collections::Class c;
}
