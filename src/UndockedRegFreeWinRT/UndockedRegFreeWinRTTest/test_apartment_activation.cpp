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

using namespace Microsoft::WRL;
using namespace ABI::Windows::Foundation;
using namespace Microsoft::WRL::Wrappers;

TEST_CASE("Test STA activation")
{
    winrt::init_apartment(winrt::apartment_type::single_threaded);
    winrt::TestComponent::Class c;
    RoActivateInstance(HStringReference(L"TestComponent.Class").Get(), (IInspectable**)winrt::put_abi(c));
    REQUIRE(c.Apartment() == APTTYPE_MAINSTA);
    winrt::clear_factory_cache();
    winrt::uninit_apartment();
}

TEST_CASE("Test MTA activation")
{
    winrt::init_apartment();
    winrt::TestComponent::Class c;
    RoActivateInstance(HStringReference(L"TestComponent.Class").Get(), (IInspectable**)winrt::put_abi(c));
    REQUIRE(c.Apartment() == APTTYPE_MTA);
    winrt::clear_factory_cache();
    winrt::uninit_apartment();
}
/*
TEST_CASE("Test cross apartment MTA activation")
{
    ExtRoLoadCatalog(L"manifestTestMTA.manifest");
    winrt::init_apartment(winrt::apartment_type::single_threaded);
    winrt::TestComponent::Class c;
    RoActivateInstance(HStringReference(L"TestComponent.Class").Get(), (IInspectable**)winrt::put_abi(c));
    REQUIRE(c.Apartment() == APTTYPE_MTA);
    winrt::clear_factory_cache();
    winrt::uninit_apartment();
}

TEST_CASE("Test block STA to MTA activation")
{
    ExtRoLoadCatalog(L"manifestTestSTA.manifest");
    winrt::init_apartment();
    bool failed = false;
    winrt::TestComponent::Class c;
    REQUIRE(RoActivateInstance(HStringReference(L"TestComponent.Class").Get(), (IInspectable**)winrt::put_abi(c)) == RO_E_UNSUPPORTED_FROM_MTA);
    winrt::clear_factory_cache();
    winrt::uninit_apartment();
}
*/
TEST_CASE("Test get metadata file")
{
    HString result;
    REQUIRE(RoGetMetaDataFile(HStringReference(L"TestComponent").Get(), nullptr, result.GetAddressOf(), nullptr, nullptr) == S_OK);
    REQUIRE(wcsstr(WindowsGetStringRawBuffer(result.Get(), 0), L"TestComponent.winmd") != nullptr);
}
