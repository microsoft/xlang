#pragma once

#include "Class.g.h"
#include <combaseapi.h>

namespace winrt::regfree_winrt_test_component::implementation
{
    struct Class : ClassT<Class>
    {
        Class() = default;

        int32_t MyProperty();
        void MyProperty(int32_t value);
		int32_t GetApartmentType();

	private:
		int32_t property;
    };
}

namespace winrt::regfree_winrt_test_component::factory_implementation
{
    struct Class : ClassT<Class, implementation::Class>
    {
    };
}
