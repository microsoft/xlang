#pragma once

#include "TestStaticLibrary4Class.g.h"

namespace winrt::TestStaticLibrary4::implementation
{
    struct TestStaticLibrary4Class : TestStaticLibrary4ClassT<TestStaticLibrary4Class>
    {
        TestStaticLibrary4Class() = default;

        int32_t MyProperty();
        void MyProperty(int32_t value);
    };
}

namespace winrt::TestStaticLibrary4::factory_implementation
{
    struct TestStaticLibrary4Class : TestStaticLibrary4ClassT<TestStaticLibrary4Class, implementation::TestStaticLibrary4Class>
    {
    };
}
