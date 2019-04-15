cmake_minimum_required(VERSION 3.9)

# The Microsoft.Windows.CppWinRT NuGet package is only targeted at Visual Studio (on Windows)
if (WIN32 AND ("$ENV{VSCMD_ARG_TGT_ARCH}" STREQUAL "x86"))

file(TO_NATIVE_PATH "${CMAKE_BINARY_DIR}/build_tools/nuget.exe" nuget_exe)
file(TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}" cppwinrt_nupkg_dir)
file(TO_NATIVE_PATH "${cppwinrt_nupkg_dir}/Microsoft.Windows.CppWinRT.${XLANG_BUILD_VERSION}.nupkg" cppwinrt_nupkg)

get_target_property(cppwinrt_exe cppwinrt "cppwinrt_exe")

file(DOWNLOAD https://dist.nuget.org/win-x86-commandline/latest/nuget.exe ${nuget_exe})

add_custom_command(OUTPUT ${cppwinrt_nupkg}
    COMMAND ${nuget_exe} pack ${CMAKE_CURRENT_SOURCE_DIR}/Microsoft.Windows.CppWinRT.nuspec -Properties cppwinrt_exe=${cppwinrt_exe}; -Version ${XLANG_BUILD_VERSION} -OutputDirectory ${CMAKE_CURRENT_BINARY_DIR} -NonInteractive -Verbosity Detailed
    DEPENDS ${XLANG_BUILD_VERSION_h} ${CMAKE_CURRENT_SOURCE_DIR}/Microsoft.Windows.CppWinRT.nuspec
)

add_custom_target(make_cppwinrt_nupkg ALL DEPENDS ${cppwinrt_nupkg} cppwinrt)

set_target_properties(make_cppwinrt_nupkg PROPERTIES "cppwinrt_nupkg_dir" ${cppwinrt_nupkg_dir})

endif()