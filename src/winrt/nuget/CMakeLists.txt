cmake_minimum_required(VERSION 3.9)

# The Microsoft.Windows.CppWinRT NuGet package is only targeted at Visual Studio (on Windows)
if (WIN32 AND ("$ENV{VSCMD_ARG_TGT_ARCH}" STREQUAL "x86"))

file(TO_NATIVE_PATH "${CMAKE_BINARY_DIR}/build_tools/nuget.exe" nuget_exe)
file(TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}" regfree_winrt_nupkg_dir)
file(TO_NATIVE_PATH "${regfree_winrt_nupkg_dir}/Microsoft.Windows.RegFreeWinRT.nupkg" regfree_winrt_nupkg)

file(DOWNLOAD https://dist.nuget.org/win-x86-commandline/latest/nuget.exe ${nuget_exe})

add_custom_command(OUTPUT ${regfree_winrt_nupkg}
    COMMAND ${nuget_exe} pack ${CMAKE_CURRENT_SOURCE_DIR}/Microsoft.Windows.RegFreeWinrt.nuspec -Properties winrtact=${CMAKE_CURRENT_BINARY_DIR}/../activation/winrtact.dll -Version ${REGFREE_WINRT_BUILD_VERSION} -OutputDirectory ${CMAKE_CURRENT_BINARY_DIR} -NonInteractive -Verbosity Detailed
)

add_custom_target(make_cppwinrt_nupkg ALL DEPENDS cppwinrt ${cppwinrt_nupkg} cppwinrt_fast_fwd)

set_target_properties(make_cppwinrt_nupkg PROPERTIES "cppwinrt_nupkg_dir" ${cppwinrt_nupkg_dir})
set_target_properties(make_cppwinrt_nupkg PROPERTIES "cppwinrt_nupkg" ${cppwinrt_nupkg})

endif()

