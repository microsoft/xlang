#pragma once
#include "Component.Edge.OneClass.g.h"

namespace winrt::Component::Edge::implementation
{
    struct OneClass : OneClassT<OneClass>
    {
        OneClass() = default;

        OneClass(int32_t a)
        {
            m_first = a;
        }

        void Method(int32_t a)
        {
            m_first = a;
        }

        static void StaticMethod(int32_t a)
        {
            m_first = a;
        }

        int32_t First()
        {
            return m_first;
        }

        inline static int32_t m_first;
    };
}
namespace winrt::Component::Edge::factory_implementation
{
    struct OneClass : OneClassT<OneClass, implementation::OneClass>
    {
    };
}
