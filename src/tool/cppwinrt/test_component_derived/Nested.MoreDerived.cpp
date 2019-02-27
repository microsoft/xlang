#include "pch.h"
#include "Nested.MoreDerived.h"

namespace winrt::test_component_derived::Nested::implementation
{
    MoreDerived::MoreDerived(hstring const& name)
    {
        throw hresult_not_implemented();
    }
    void MoreDerived::MoreDerivedMethod()
    {
        throw hresult_not_implemented();
    }
}
