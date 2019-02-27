#include "pch.h"
#include "Nested.Derived.h"

namespace winrt::test_component_derived::Nested::implementation
{
    Derived::Derived(hstring const& name)
    {
        throw hresult_not_implemented();
    }
    void Derived::DerivedMethod()
    {
        throw hresult_not_implemented();
    }
}
