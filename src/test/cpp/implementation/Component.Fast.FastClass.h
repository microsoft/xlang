#pragma once
#include "Component.Fast.FastClass.g.h"

namespace winrt::Component::Fast::implementation
{
    struct FastClass : FastClassT<FastClass>
    {
        FastClass() = default;

        hstring First()
        {
            return m_first;
        }

        hstring Second()
        {
            return m_second;
        }

        hstring Third()
        {
            return m_third;
        }

        hstring Fourth()
        {
            return m_fourth;
        }

        hstring NotExclusive()
        {
            return m_not_exclusive;
        }

        static hstring StaticMethod()
        {
            return m_static;
        };

        hstring m_first{ L"Fast.First" };
        hstring m_second{ L"Fast.Second" };
        hstring m_third{ L"Fast.Third" };
        hstring m_fourth{ L"Fast.Fourth" };
        hstring m_not_exclusive{ L"Fast.NotExclusive" };
        inline static hstring m_static{ L"Fast.Static" };
    };
}
namespace winrt::Component::Fast::factory_implementation
{
    struct FastClass : FastClassT<FastClass, implementation::FastClass>
    {
    };
}

