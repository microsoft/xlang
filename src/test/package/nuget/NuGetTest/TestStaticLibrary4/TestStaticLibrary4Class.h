#pragma once

#include "TestStaticLibrary4Class.g.h"

namespace winrt::TestStaticLibrary4::implementation
{
    struct TestStaticLibrary4Class : TestStaticLibrary4ClassT<TestStaticLibrary4Class>
    {
        TestStaticLibrary4Class() = default;

        void Test();
    };
}

namespace winrt::TestStaticLibrary4::factory_implementation
{
    struct TestStaticLibrary4Class : TestStaticLibrary4ClassT<TestStaticLibrary4Class, implementation::TestStaticLibrary4Class>
    {
    };
}
