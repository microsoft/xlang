#pragma once
#include "TestStaticLibrary1Class.g.h"

namespace winrt::TestStaticLibrary1::implementation
{
    struct TestStaticLibrary1Class : TestStaticLibrary1ClassT<TestStaticLibrary1Class>
    {
        TestStaticLibrary1Class() = default;

        void Test();
    };
}
namespace winrt::TestStaticLibrary1::factory_implementation
{
    struct TestStaticLibrary1Class : TestStaticLibrary1ClassT<TestStaticLibrary1Class, implementation::TestStaticLibrary1Class>
    {
    };
}
