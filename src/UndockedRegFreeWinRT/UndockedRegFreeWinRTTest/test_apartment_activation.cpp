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

TEST_CASE("Undocked Regfree WinRT Activation")
{
    RegFreeWinRTInitializeForTest();

    //SECTION("Both To Current STA")
    //{
    //    winrt::init_apartment(winrt::apartment_type::single_threaded);
    //    winrt::TestComponent::ClassBoth c;
    //    RoActivateInstance(HStringReference(L"TestComponent.ClassBoth").Get(), (IInspectable**)winrt::put_abi(c));
    //    REQUIRE(c.Apartment() == APTTYPE_MAINSTA);
    //    winrt::clear_factory_cache();
    //    winrt::uninit_apartment();
    //}
    //SECTION("Both To Current MTA")
    //{
    //    winrt::init_apartment();
    //    winrt::TestComponent::ClassBoth c;
    //    RoActivateInstance(HStringReference(L"TestComponent.ClassBoth").Get(), (IInspectable**)winrt::put_abi(c));
    //    REQUIRE(c.Apartment() == APTTYPE_MTA);
    //    winrt::clear_factory_cache();
    //    winrt::uninit_apartment();
    //}
    SECTION("Cross Apartment MTA Activation")
    {
        std::cout << "1" << std::endl;
        winrt::init_apartment(winrt::apartment_type::single_threaded);
        std::cout << "2" << std::endl;
        winrt::com_ptr<IInspectable> instance;
        std::cout << "3" << std::endl;
        winrt::TestComponent::ClassMta c;
        std::cout << "4" << std::endl;
        REQUIRE(RoActivateInstance(HStringReference(L"TestComponent.ClassMta").Get(), reinterpret_cast<::IInspectable**>(winrt::put_abi(instance))) ==  S_OK);
        std::cout << "5" << std::endl;
        instance.as(c);
        std::cout << "6" << std::endl;
        REQUIRE(c.Apartment() == APTTYPE_MTA);
        std::cout << "7" << std::endl;
        winrt::clear_factory_cache();
        winrt::uninit_apartment();
    }
    //SECTION("BLOCK STA To Current MTA")
    //{
    //    winrt::init_apartment();
    //    winrt::TestComponent::ClassSta c;
    //    REQUIRE(RoActivateInstance(HStringReference(L"TestComponent.ClassSta").Get(), (IInspectable**)winrt::put_abi(c)) == RO_E_UNSUPPORTED_FROM_MTA);
    //    winrt::clear_factory_cache();
    //    winrt::uninit_apartment();
    //}
    //SECTION("Test Get Metadata File")
    //{
    //    HString result;
    //    REQUIRE(RoGetMetaDataFile(HStringReference(L"TestComponent").Get(), nullptr, result.GetAddressOf(), nullptr, nullptr) == S_OK);
    //    REQUIRE(wcsstr(WindowsGetStringRawBuffer(result.Get(), 0), L"TestComponent.winmd") != nullptr);
    //}

    RegFreeWinRTUninitializeForTest();
}
