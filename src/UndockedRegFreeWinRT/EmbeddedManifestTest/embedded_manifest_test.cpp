#include "pch.h"
#include <iostream>

#include <Windows.h>
#include <roapi.h>
#include <rometadataresolution.h>
#include <windows.foundation.h>
#include <stdio.h>
#include <wrl\client.h>
#include <wrl\wrappers\corewrappers.h>
#include "winrt\TestComponent.h"

#include "../UndockedRegFreeWinRT/extwinrt.h"

using namespace Microsoft::WRL;
using namespace ABI::Windows::Foundation;
using namespace Microsoft::WRL::Wrappers;

TEST_CASE("Embedded Manifest Test")
{
    SECTION("Activate TestComponent.Both")
    {
        winrt::init_apartment(winrt::apartment_type::single_threaded);
        winrt::TestComponent::ClassBoth c;
        REQUIRE(c.Apartment() == APTTYPE_MAINSTA);
        winrt::clear_factory_cache();
        winrt::uninit_apartment();
    }
    SECTION("Activate TestComponent.Both")
    {
        winrt::init_apartment(winrt::apartment_type::single_threaded);
        winrt::TestComponent::ClassSta c;
        REQUIRE(c.Apartment() == APTTYPE_MAINSTA);
        winrt::clear_factory_cache();
        winrt::uninit_apartment();
    }
    SECTION("Activate TestComponent.MTA")
    {
        winrt::init_apartment(winrt::apartment_type::single_threaded);
        winrt::TestComponent::ClassMta c;
        REQUIRE(c.Apartment() == APTTYPE_MTA);
        winrt::clear_factory_cache();
        winrt::uninit_apartment();
    }
}
