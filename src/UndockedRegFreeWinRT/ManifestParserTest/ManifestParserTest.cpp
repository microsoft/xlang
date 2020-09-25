// ManifestParserTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "..\UndockedRegFreeWinRT\catalog.h"
#include "..\UndockedRegFreeWinRT\catalog.cpp"
#include "..\UndockedRegFreeWinRT\typeresolution.h"
#include "..\UndockedRegFreeWinRT\typeresolution.cpp"
#include "pch.h"

#define RegFreeWinrtInprocComponents1 L"RegFreeWinrtInprocComponents.1.dll"
#define FakeDllName L"FakeDllName.dll"
#define OtherFakeDllName L"OtherFakeDllName.dll"

TEST_CASE("Multiple activatableClass")
{
    g_types.clear();
    PCWSTR exeFilePath = nullptr;
    UndockedRegFreeWinRT::GetProcessExeDir(&exeFilePath);
    std::wstring test1 = std::wstring(exeFilePath) + L"\\winrtActivation.dll1.manifest";
    LoadFromSxSManifest(test1.c_str());
    std::wstring moduleName = RegFreeWinrtInprocComponents1;
    REQUIRE(g_types.size() == 4);
    auto component_iter = g_types.find(L"RegFreeWinrt.1.threading_both");
    REQUIRE(component_iter != g_types.end());
    REQUIRE(component_iter->second->module_name == moduleName);

    component_iter = g_types.find(L"RegFreeWinrt.1.threading_mta");
    REQUIRE(component_iter != g_types.end());
    REQUIRE(component_iter->second->module_name == moduleName);

    component_iter = g_types.find(L"RegFreeWinrt.1.threading_sta");
    REQUIRE(component_iter != g_types.end());
    REQUIRE(component_iter->second->module_name == moduleName);

    component_iter = g_types.find(L"RegFreeWinrt.SharedName.threading_both");
    REQUIRE(component_iter != g_types.end());
    REQUIRE(component_iter->second->module_name == moduleName);
}

TEST_CASE("Multiple file")
{
    g_types.clear();
    PCWSTR exeFilePath = nullptr;
    UndockedRegFreeWinRT::GetProcessExeDir(&exeFilePath);
    std::wstring test1 = std::wstring(exeFilePath) + L"\\basicParse.Positive.manifest";
    LoadFromSxSManifest(test1.c_str());
    std::wstring moduleName = FakeDllName;
    REQUIRE(g_types.size() == 6);
    auto component_iter = g_types.find(L"FakeActivatableClass");
    REQUIRE(component_iter != g_types.end());
    REQUIRE(component_iter->second->module_name == moduleName);

    component_iter = g_types.find(L"FakeActivatableClass2");
    REQUIRE(component_iter != g_types.end());
    REQUIRE(component_iter->second->module_name == moduleName);

    component_iter = g_types.find(L"FakeActivatableClass3");
    REQUIRE(component_iter != g_types.end());
    REQUIRE(component_iter->second->module_name == moduleName);

    component_iter = g_types.find(L"FakeActivatableClass4");
    REQUIRE(component_iter != g_types.end());
    REQUIRE(component_iter->second->module_name == moduleName);

    moduleName = OtherFakeDllName;
    component_iter = g_types.find(L"OtherFakeActivatableClass");
    REQUIRE(component_iter != g_types.end());
    REQUIRE(component_iter->second->module_name == moduleName);

    component_iter = g_types.find(L"OtherFakeActivatableClass2");
    REQUIRE(component_iter != g_types.end());
    REQUIRE(component_iter->second->module_name == moduleName);
}

