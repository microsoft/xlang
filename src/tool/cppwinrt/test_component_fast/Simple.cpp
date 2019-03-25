#include "pch.h"
#include "Simple.h"
#include "Simple.g.cpp"

namespace winrt::test_component_fast::implementation
{
    hstring Simple::Method1()
    {
        return L"Method1";
    }
    hstring Simple::Method2()
    {
        return L"Method2";
    }
    hstring Simple::Method3()
    {
        return L"Method3";
    }
}
