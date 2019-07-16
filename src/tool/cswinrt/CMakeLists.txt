# The C# WinRT compiler is only targeted at Visual Studio (on Windows)
if (WIN32)

project(cswinrt)

add_executable(cswinrt "")
target_sources(cswinrt PUBLIC main.cpp pch.cpp "${PROJECT_BINARY_DIR}/strings.cpp")
target_include_directories(cswinrt PUBLIC ${XLANG_LIBRARY_PATH} ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR})

GENERATE_STRING_LITERAL_FILES("${PROJECT_SOURCE_DIR}/strings/*" "strings" "cswinrt::strings" cswinrt)

if (WIN32)
    TARGET_CONFIG_MSVC_PCH(cswinrt pch.cpp pch.h)
    target_link_libraries(cswinrt windowsapp ole32 shlwapi)
else()
    target_link_libraries(cswinrt c++ c++abi c++experimental)
    target_link_libraries(cswinrt -lpthread)
endif()

set(build_testcomp msbuild testcomp/TestComp.vcxproj /nologo /m /p:Configuration=${CMAKE_BUILD_TYPE},Platform=$ENV{VSCMD_ARG_TGT_ARCH},SolutionDir=${CMAKE_CURRENT_SOURCE_DIR}/)

if ("$ENV{VSCMD_ARG_TGT_ARCH}" STREQUAL "x86")
set(ProgramFiles "PROGRAMFILES(X86)")
else()
set(ProgramFiles "PROGRAMFILES")
endif()
set(dotnet_exe "$ENV{${ProgramFiles}}/dotnet/dotnet.exe")

set(run_unittest ${dotnet_exe} test --logger xunit$<SEMICOLON>LogFilePath=${CMAKE_BINARY_DIR}/test_cswinrt_unittest.xml unittest/UnitTest.csproj /nologo /m /p:Configuration=${CMAKE_BUILD_TYPE},Platform=$ENV{VSCMD_ARG_TGT_ARCH},SolutionDir=${CMAKE_CURRENT_SOURCE_DIR}/)

file(TO_NATIVE_PATH "${CMAKE_BINARY_DIR}/build_tools/nuget.exe" nuget_exe)
file(DOWNLOAD https://dist.nuget.org/win-x86-commandline/latest/nuget.exe ${nuget_exe})

add_custom_target(cswinrt_test ALL DEPENDS cswinrt
    COMMAND "${CMAKE_CURRENT_BINARY_DIR}/cswinrt.exe" -input local -output "${CMAKE_CURRENT_BINARY_DIR}/output" -include Windows.Foundation -exclude Windows.Foundation.Metadata -exclude Windows.Foundation.Diagnostics -include Windows.Data.Json.Foundation.Metadata
    COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR} 
	COMMAND ${nuget_exe} restore
    COMMAND ${build_testcomp} 
    COMMAND ${run_unittest} 
)

endif()
