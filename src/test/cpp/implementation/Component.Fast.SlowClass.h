#pragma once
#include "Component.Fast.SlowClass.g.h"

namespace winrt::Component::Fast::implementation
{
    struct SlowClass : SlowClassT<SlowClass>
    {
        SlowClass() = default;

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

        hstring m_first{ L"Slow.First" };
        hstring m_second{ L"Slow.Second" };
        hstring m_third{ L"Slow.Third" };
        hstring m_fourth{ L"Slow.Fourth" };
        hstring m_not_exclusive{ L"Slow.NotExclusive" };
        inline static hstring m_static{ L"Slow.Static" };
    };
}
namespace winrt::Component::Fast::factory_implementation
{
    struct SlowClass : SlowClassT<SlowClass, implementation::SlowClass>
    {
    };
}
