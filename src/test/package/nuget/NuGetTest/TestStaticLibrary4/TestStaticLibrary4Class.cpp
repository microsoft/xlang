#include "pch.h"
#include "TestStaticLibrary4Class.h"
#if __has_include("TestStaticLibrary4Class.g.cpp")
#include "TestStaticLibrary4Class.g.cpp"
#endif

using namespace winrt;

namespace winrt::TestStaticLibrary4::implementation
{
    void TestStaticLibrary4Class::Test()
    {
    }
}
