cmake_minimum_required(VERSION 3.9)

# The Microsoft.Windows.CppWinRT NuGet package is only targeted at Visual Studio (on Windows)
if (WIN32)

file(TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}" cppwinrt_nupkg_dir)
file(TO_NATIVE_PATH "${cppwinrt_nupkg_dir}/Microsoft.Windows.CppWinRT.${cppwinrt_version}.nupkg" cppwinrt_nupkg)

add_custom_command(OUTPUT ${cppwinrt_nupkg}
    COMMAND nuget pack ${CMAKE_CURRENT_SOURCE_DIR}/Microsoft.Windows.CppWinRT.nuspec -Version ${cppwinrt_version} -OutputDirectory ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${cppwinrt_version_h}
)

add_custom_target(make_cppwinrt_nupkg ALL DEPENDS ${cppwinrt_nupkg})

set_target_properties(make_cppwinrt_nupkg PROPERTIES "cppwinrt_nupkg_dir" ${cppwinrt_nupkg_dir})

endif(WIN32)