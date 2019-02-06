#include "pch.h"
#include "Component.Composable.Base.h"
#include "Component.Composable.Base.g.cpp"

namespace winrt::Component::Composable::implementation
{
    hstring Base::BaseMethod()
    {
        return L"Base";
    }
}
