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

TEST_CASE("Normal Activation")
{
	ExtRoLoadCatalog(L"manifesttest1.txt");
	HRESULT hr = S_OK;
	CoInitialize(nullptr);
	{
		ComPtr<IInspectable> instance;
		REQUIRE(RoActivateInstance(HStringReference(L"test_component.Class").Get(), &instance) == S_OK);

		HString result;
		REQUIRE(instance->GetRuntimeClassName(result.GetAddressOf()) == S_OK);
		REQUIRE(result == HStringReference(L"test_component.Class").Get());
	}
	CoUninitialize();
}

TEST_CASE("Block MTA to STA activation")
{
	ExtRoLoadCatalog(L"manifesttest1.txt");
	HRESULT hr = S_OK;
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	{
		ComPtr<IInspectable> instance;
		REQUIRE(RoActivateInstance(HStringReference(L"test_component.Class").Get(), &instance) == S_OK);

		HString result;
		REQUIRE(instance->GetRuntimeClassName(result.GetAddressOf()) == S_OK);
		REQUIRE(result == HStringReference(L"test_component.Class").Get());
	}
	CoUninitialize();
}
