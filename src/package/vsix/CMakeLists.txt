cmake_minimum_required(VERSION 3.9)

# The C++/WinRT VSIX package is only targeted at Visual Studio (on Windows)
if (WIN32 AND vssdk_installed)

get_target_property(cppwinrt_natvis_dir cppwinrtvisualizer "cppwinrt_natvis_dir")
get_target_property(cppwinrt_nupkg_dir make_cppwinrt_nupkg "cppwinrt_nupkg_dir")

file(TO_NATIVE_PATH ${CMAKE_CURRENT_BINARY_DIR}/Microsoft.Windows.CppWinRT.vsix cppwinrt_vsix)
file(TO_NATIVE_PATH "${cppwinrt_nupkg_dir}/Microsoft.Windows.CppWinRT.${cppwinrt_version}.nupkg" cppwinrt_nupkg)

set(build_vsix msbuild ${CMAKE_CURRENT_SOURCE_DIR}/vsix.csproj /nologo /m /p:Configuration=${CMAKE_BUILD_TYPE},Platform=$ENV{VSCMD_ARG_TGT_ARCH})

add_custom_command(OUTPUT ${cppwinrt_vsix}
    COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND ${build_vsix} /p:CppWinRTVersion=${cppwinrt_version} /t:VersionTemplates
    COMMAND ${CMAKE_COMMAND} -E copy ../LICENSE .
    COMMAND ${build_vsix} /p:CppWinRTVersion=${cppwinrt_version},NatvisDir=${cppwinrt_natvis_dir},NupkgDir=${cppwinrt_nupkg_dir}
    COMMAND ${CMAKE_COMMAND} -E remove LICENSE
    COMMAND ${build_vsix} /p:CppWinRTVersion=1.0.0.0 /t:VersionTemplates
    DEPENDS ${cppwinrt_version_h} cppwinrtvisualizer ${cppwinrt_nupkg}
)

add_custom_target(make_cppwinrt_vsix ALL DEPENDS ${cppwinrt_vsix})

endif()