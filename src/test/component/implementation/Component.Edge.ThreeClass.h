#pragma once
#include "Component.Edge.ThreeClass.g.h"

namespace winrt::Component::Edge::implementation
{
    struct ThreeClass : ThreeClassT<ThreeClass>
    {
        ThreeClass() = default;

        ThreeClass(int32_t a, int32_t b, int32_t c)
        {
            StaticMethod(a, b, c);
        }

        void Method(int32_t a, int32_t b, int32_t c)
        {
            StaticMethod(a, b, c);
        }

        static void StaticMethod(int32_t a, int32_t b, int32_t c)
        {
            m_first = a;
            m_second = b;
            m_third = c;
        }

        int32_t First()
        {
            return m_first;
        }

        int32_t Second()
        {
            return m_second;
        }

        int32_t Third()
        {
            return m_third;
        }

        inline static int32_t m_first;
        inline static int32_t m_second;
        inline static int32_t m_third;
    };
}
namespace winrt::Component::Edge::factory_implementation
{
    struct ThreeClass : ThreeClassT<ThreeClass, implementation::ThreeClass>
    {
    };
}
