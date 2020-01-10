#include "pch.h"
#include <iostream>

#include <Windows.h>
#include <roapi.h>
#include <rometadataresolution.h>
#include <windows.foundation.h>
#include <stdio.h>
#include <wrl\client.h>
#include <wrl\wrappers\corewrappers.h>
#include <extwinrt.h>

using namespace Microsoft::WRL;
using namespace ABI::Windows::Foundation;
using namespace Microsoft::WRL::Wrappers;

TEST_CASE("Test STA activation")
{
	ExtRoLoadCatalog(L"manifesttest1.txt");
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
	ExtRoLoadCatalog(L"manifesttest1.txt");
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
	ExtRoLoadCatalog(L"manifesttest1.txt");
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
