#include "pch.h"
#include <iostream>

#include <Windows.h>
#include <roapi.h>
#include <rometadataresolution.h>
#include <windows.foundation.h>
#include <stdio.h>
#include <wrl\client.h>
#include <wrl\wrappers\corewrappers.h>

#include "../UndockedRegFreeWinRT/extwinrt.h"

using namespace Microsoft::WRL;
using namespace ABI::Windows::Foundation;
using namespace Microsoft::WRL::Wrappers;

TEST_CASE("Test STA activation")
{
    ExtRoLoadCatalog(L"manifestTestBoth.manifest");
    HRESULT hr = S_OK;
    CoInitialize(nullptr);
    {
        ComPtr<IInspectable> instance;
        REQUIRE(RoActivateInstance(HStringReference(L"RegFreeWinRtTest.TestComp").Get(), &instance) == S_OK);

        HString result;
        REQUIRE(instance->GetRuntimeClassName(result.GetAddressOf()) == S_OK);
        REQUIRE(result == HStringReference(L"RegFreeWinRtTest.TestComp").Get());

        ComPtr<IStringable> stringUri;
        REQUIRE(instance->QueryInterface<IStringable>(&stringUri) == S_OK);

        HString outputString;
        stringUri->ToString(outputString.GetAddressOf());
        REQUIRE(outputString == HStringReference(L"MAIN_STA").Get());
    }
    CoUninitialize();
}

TEST_CASE("Test MTA activation")
{
    ExtRoLoadCatalog(L"manifestTestBoth.manifest");
    HRESULT hr = S_OK;
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    {
        ComPtr<IInspectable> instance;
        REQUIRE(RoActivateInstance(HStringReference(L"RegFreeWinRtTest.TestComp").Get(), &instance) == S_OK);

        HString result;
        REQUIRE(instance->GetRuntimeClassName(result.GetAddressOf()) == S_OK);
        REQUIRE(result == HStringReference(L"RegFreeWinRtTest.TestComp").Get());

        ComPtr<IStringable> stringUri;
        REQUIRE(instance->QueryInterface<IStringable>(&stringUri) == S_OK);

        HString outputString;
        stringUri->ToString(outputString.GetAddressOf());
        REQUIRE(outputString == HStringReference(L"MTA").Get());
    }
    CoUninitialize();
}

TEST_CASE("Test cross apartment MTA activation")
{
    ExtRoLoadCatalog(L"manifestTestMTA.manifest");
    HRESULT hr = S_OK;
    CoInitialize(nullptr);
    {
        ComPtr<IInspectable> instance;
        REQUIRE(RoActivateInstance(HStringReference(L"RegFreeWinRtTest.TestComp").Get(), &instance) == S_OK);

        HString result;
        REQUIRE(instance->GetRuntimeClassName(result.GetAddressOf()) == S_OK);
        REQUIRE(result == HStringReference(L"RegFreeWinRtTest.TestComp").Get());

        ComPtr<IStringable> stringUri;
        REQUIRE(instance->QueryInterface<IStringable>(&stringUri) == S_OK);

        HString outputString;
        stringUri->ToString(outputString.GetAddressOf());
        REQUIRE(outputString == HStringReference(L"MTA").Get());
    }
    CoUninitialize();
}

TEST_CASE("Test block STA to MTA activation")
{
    ExtRoLoadCatalog(L"manifestTestSTA.manifest");
    HRESULT hr = S_OK;
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    {
        ComPtr<IInspectable> instance;
        REQUIRE(RoActivateInstance(HStringReference(L"RegFreeWinRtTest.TestComp").Get(), &instance) == RO_E_UNSUPPORTED_FROM_MTA);
    }
    CoUninitialize();
}

TEST_CASE("Test get metadata file")
{
    ExtRoLoadCatalog(L"manifestTestBoth.manifest");
    HRESULT hr = S_OK;
    CoInitialize(nullptr);
    {
        HString result;

        hr = RoGetMetaDataFile(HStringReference(L"RegFreeWinRtTest.TestComp").Get(), nullptr, result.GetAddressOf(), nullptr, nullptr);

        REQUIRE(RoGetMetaDataFile(HStringReference(L"RegFreeWinRtTest.TestComp").Get(), nullptr, result.GetAddressOf(), nullptr, nullptr) == S_OK);
        REQUIRE(wcsstr(WindowsGetStringRawBuffer(result.Get(), 0), L"RegFreeWinRtTest.winmd") != nullptr);
    }
    CoUninitialize();
}
