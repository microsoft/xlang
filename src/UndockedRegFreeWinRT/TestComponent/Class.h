#pragma once

#include "ClassBoth.g.h"
#include "ClassSta.g.h"
#include "ClassMta.g.h"

namespace winrt::TestComponent::implementation
{
    struct ClassBoth : ClassBothT<ClassBoth>
    {
        ClassBoth() = default;

        int32_t Apartment();
        void Apartment(int32_t value);
    };

    struct ClassSta : ClassStaT<ClassSta>
    {
        ClassSta() = default;

        int32_t Apartment();
        void Apartment(int32_t value);
    };

    struct ClassMta : ClassMtaT<ClassMta>
    {
        ClassMta() = default;

        int32_t Apartment();
        void Apartment(int32_t value);
    };
}

namespace winrt::TestComponent::factory_implementation
{
    struct ClassBoth : ClassBothT<ClassBoth, implementation::ClassBoth>
    {
    };

    struct ClassSta : ClassStaT<ClassSta, implementation::ClassSta>
    {
    };

    struct ClassMta : ClassMtaT<ClassMta, implementation::ClassMta>
    {
    };
}
