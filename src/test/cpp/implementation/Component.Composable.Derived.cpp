#include "pch.h"
#include "Component.Composable.Derived.h"
#include "Component.Composable.Derived.g.cpp"

namespace winrt::Component::Composable::implementation
{
    hstring Derived::DerivedMethod()
    {
        return L"Derived";
    }
}
