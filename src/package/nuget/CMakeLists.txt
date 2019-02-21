cmake_minimum_required(VERSION 3.9)

# The Microsoft.Windows.CppWinRT NuGet package is only targeted at Visual Studio (on Windows)
if (WIN32)

file(TO_NATIVE_PATH "${CMAKE_BINARY_DIR}/build_tools/nuget.exe" nuget_exe)
file(TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}" cppwinrt_nupkg_dir)
file(TO_NATIVE_PATH "${cppwinrt_nupkg_dir}/Microsoft.Windows.CppWinRT.${XLANG_BUILD_VERSION}.nupkg" cppwinrt_nupkg)

file(DOWNLOAD https://dist.nuget.org/win-x86-commandline/latest/nuget.exe ${nuget_exe})

add_custom_command(OUTPUT ${cppwinrt_nupkg}
    COMMAND ${nuget_exe} pack ${CMAKE_CURRENT_SOURCE_DIR}/Microsoft.Windows.CppWinRT.nuspec -Version ${XLANG_BUILD_VERSION} -OutputDirectory ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${XLANG_BUILD_VERSION_h}
)

add_custom_target(make_cppwinrt_nupkg ALL DEPENDS ${cppwinrt_nupkg})

set_target_properties(make_cppwinrt_nupkg PROPERTIES "cppwinrt_nupkg_dir" ${cppwinrt_nupkg_dir})

endif(WIN32)