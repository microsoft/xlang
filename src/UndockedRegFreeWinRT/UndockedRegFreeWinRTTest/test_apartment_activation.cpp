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

TEST_CASE("Test STA activation")
{
    RegFreeWinRTInitializeForTest();

    winrt::init_apartment(winrt::apartment_type::single_threaded);
    winrt::TestComponent::ClassBoth c;
    RoActivateInstance(HStringReference(L"TestComponent.ClassBoth").Get(), (IInspectable**)winrt::put_abi(c));
    REQUIRE(c.Apartment() == APTTYPE_MAINSTA);
    winrt::clear_factory_cache();
    winrt::uninit_apartment();

    RegFreeWinRTUninitializeForTest();
}

TEST_CASE("Test MTA activation")
{
    RegFreeWinRTInitializeForTest();

    winrt::init_apartment();
    winrt::TestComponent::ClassBoth c;
    RoActivateInstance(HStringReference(L"TestComponent.ClassBoth").Get(), (IInspectable**)winrt::put_abi(c));
    REQUIRE(c.Apartment() == APTTYPE_MTA);
    winrt::clear_factory_cache();
    winrt::uninit_apartment();

    RegFreeWinRTUninitializeForTest();
}
/*
TEST_CASE("Test cross apartment MTA activation")
{
    winrt::init_apartment(winrt::apartment_type::single_threaded);
    winrt::TestComponent::ClassMta c;
    RoActivateInstance(HStringReference(L"TestComponent.ClassMta").Get(), (IInspectable**)winrt::put_abi(c));
    REQUIRE(c.Apartment() == APTTYPE_MTA);
    winrt::clear_factory_cache();
    winrt::uninit_apartment();
}
*/
TEST_CASE("Test block STA to MTA activation")
{
    RegFreeWinRTInitializeForTest();

    winrt::init_apartment();
    winrt::TestComponent::ClassSta c;
    REQUIRE(RoActivateInstance(HStringReference(L"TestComponent.ClassSta").Get(), (IInspectable**)winrt::put_abi(c)) == RO_E_UNSUPPORTED_FROM_MTA);
    winrt::clear_factory_cache();
    winrt::uninit_apartment();

    RegFreeWinRTUninitializeForTest();
}

TEST_CASE("Test get metadata file")
{
    RegFreeWinRTInitializeForTest();

    HString result;
    REQUIRE(RoGetMetaDataFile(HStringReference(L"TestComponent").Get(), nullptr, result.GetAddressOf(), nullptr, nullptr) == S_OK);
    REQUIRE(wcsstr(WindowsGetStringRawBuffer(result.Get(), 0), L"TestComponent.winmd") != nullptr);

    RegFreeWinRTUninitializeForTest();
}
