cmake_minimum_required(VERSION 3.9)

# The Microsoft.Windows.CppWinRT NuGet package is only targeted at Visual Studio (on Windows)
if (WIN32 AND ("$ENV{VSCMD_ARG_TGT_ARCH}" STREQUAL "x86"))

file(TO_NATIVE_PATH "${CMAKE_BINARY_DIR}/build_tools/nuget.exe" nuget_exe)
file(TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}" regfree_winrt_nupkg_dir)
file(TO_NATIVE_PATH "${regfree_winrt_nupkg_dir}/Microsoft.Windows.RegFreeWinRT.nupkg" regfree_winrt_nupkg)

file(DOWNLOAD https://dist.nuget.org/win-x86-commandline/latest/nuget.exe ${nuget_exe})

add_custom_command(OUTPUT ${regfree_winrt_nupkg}
    COMMAND ${nuget_exe} pack ${CMAKE_CURRENT_SOURCE_DIR}/Microsoft.Windows.RegFreeWinrt.nuspec -Properties winrtact=${CMAKE_CURRENT_BINARY_DIR}/../activation/winrtact.dll winrtactinclude=${CMAKE_CURRENT_SOURCE_DIR}/../include/extwinrt.h -Version ${REGFREE_WINRT_BUILD_VERSION} -OutputDirectory ${CMAKE_CURRENT_BINARY_DIR} -NonInteractive -Verbosity Detailed
)

#"D:\Downloads\nuget.exe" pack "C:\Users\kyawt\source\repos\xlang\src\winrt\nuget\Microsoft.Windows.RegFreeWinrt.nuspec" -Properties winrtact="C:\Users\kyawt\source\repos\xlang\src\_build\Windows\x64\Release\winrt\activation\winrtact.dll" -Properties winrtactinclude="C:\Users\kyawt\source\repos\xlang\src\winrt\include\extwinrt.h" -NonInteractive -Verbosity Detailed

endif()

