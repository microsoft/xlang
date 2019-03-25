#include "pch.h"
#include "Fast.Class.h"
#include "Fast.Class.g.cpp"

namespace winrt::test_component::Fast::implementation
{
    Class::Class()
    {
    }

    hstring Class::Class_1()
    {
        return L"one";
    }
    hstring Class::Class_2()
    {
        return L"two";
    }
    hstring Class::Class_3()
    {
        return L"three";
    }
}
