# The C# WinRT compiler is only targeted at Visual Studio (on Windows)
if (WIN32)

file(TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/cswinrt.exe" cswinrt_exe)
file(TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}" CmakeOutDir)

set(build msbuild /nologo /m /p:Configuration=${CMAKE_BUILD_TYPE},Platform=$ENV{VSCMD_ARG_TGT_ARCH},XlangBuildVersion=${XLANG_BUILD_VERSION},CmakeOutDir=${CmakeOutDir} ${CMAKE_CURRENT_SOURCE_DIR}/cswinrt.sln)

add_custom_command(OUTPUT ${cswinrt_exe}
    COMMAND ${build} /t:cswinrt
)

add_custom_target(cswinrt ALL DEPENDS ${cswinrt_exe})

set_target_properties(cswinrt PROPERTIES "cswinrt_exe" ${cswinrt_exe})

# Note: the test component is built with msbuild, but the xUnit test is built/run with dotnet
# See TestPerf\readme.txt for rationale and other test considerations
set(build_testcomp msbuild testcomp/TestComp.vcxproj /nologo /m /p:Configuration=${CMAKE_BUILD_TYPE},Platform=$ENV{VSCMD_ARG_TGT_ARCH},SolutionDir=${CMAKE_CURRENT_SOURCE_DIR}/)

if ("$ENV{VSCMD_ARG_TGT_ARCH}" STREQUAL "x86")
set(ProgramFiles "PROGRAMFILES(X86)")
else()
set(ProgramFiles "PROGRAMFILES")
endif()
set(dotnet_exe "$ENV{${ProgramFiles}}/dotnet/dotnet.exe")

# Build/Run xUnit tests, generating xml output report for Azure Devops reporting, via XunitXml.TestLogger NuGet
set(run_unittest ${dotnet_exe} test --logger xunit$<SEMICOLON>LogFilePath=${CMAKE_BINARY_DIR}/test_cswinrt_unittest.xml unittest/UnitTest.csproj /nologo /m /p:Configuration=${CMAKE_BUILD_TYPE},Platform=$ENV{VSCMD_ARG_TGT_ARCH},SolutionDir=${CMAKE_CURRENT_SOURCE_DIR}/)

file(TO_NATIVE_PATH "${CMAKE_BINARY_DIR}/build_tools/nuget.exe" nuget_exe)
file(DOWNLOAD https://dist.nuget.org/win-x86-commandline/latest/nuget.exe ${nuget_exe})

add_custom_target(cswinrt_test ALL DEPENDS cswinrt
#    COMMAND ${cswinrt_exe} -verbose -in local -out "${CMAKE_CURRENT_BINARY_DIR}/output" -include Windows.Foundation -exclude Windows.Foundation.Metadata -exclude Windows.Foundation.Diagnostics -include Windows.Data.Json.Foundation.Metadata
#    COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR} 
    COMMAND ${nuget_exe} restore
#    COMMAND ${build_testcomp} 
    COMMAND ${run_unittest} 
)

endif()
