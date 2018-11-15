#include "pch.h"
#include "Component.Composable.Derived.h"

namespace winrt::Component::Composable::implementation
{
    hstring Derived::DerivedMethod()
    {
        throw hresult_not_implemented();
    }
}
