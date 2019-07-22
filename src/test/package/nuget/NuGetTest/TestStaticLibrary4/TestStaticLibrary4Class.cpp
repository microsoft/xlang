#include "pch.h"
#include "TestStaticLibrary4Class.h"
#if __has_include("TestStaticLibrary4Class.g.cpp")
#include "TestStaticLibrary4Class.g.cpp"
#endif

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::TestStaticLibrary4::implementation
{
    int32_t TestStaticLibrary4Class::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void TestStaticLibrary4Class::MyProperty(int32_t /*value*/)
    {
        throw hresult_not_implemented();
    }
}
