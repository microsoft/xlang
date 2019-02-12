cmake_minimum_required(VERSION 3.9)

# The natvis cppwinrtvisualizer.dll is only targeted at Visual Studio (on Windows)
if (WIN32 AND vssdk_installed)

file(TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}" cppwinrt_natvis_dir)

project(cppwinrtvisualizer)

add_library(cppwinrtvisualizer SHARED)

target_sources(cppwinrtvisualizer PUBLIC pch.cpp dllmain.cpp cppwinrt_visualizer.cpp object_visualizer.cpp property_visualizer.cpp cppwinrtvisualizer.def)
TARGET_CONFIG_MSVC_PCH(cppwinrtvisualizer pch.cpp pch.h)
target_include_directories(cppwinrtvisualizer PUBLIC ${XLANG_LIBRARY_PATH} "$ENV{VSInstallDir}DIA SDK/include" "$ENV{VSInstallDir}VSSDK/VisualStudioIntegration/Common/inc")
# VSSDK headers not conformant
string(APPEND CMAKE_CXX_FLAGS " /wd4471")

target_link_libraries(cppwinrtvisualizer windowsapp shell32 advapi32 $ENV{VSInstallDir}VSSDK/VisualStudioIntegration/Common/lib/$ENV{VSCMD_ARG_TGT_ARCH}/vsdebugeng.lib)

file(TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/cppwinrtvisualizer.vsdconfig" cppwinrtvisualizer_vsdconfig)
file(TO_NATIVE_PATH "$ENV{VSInstallDir}VSSDK/VisualStudioIntegration/Tools/Bin/vsdconfigtool.exe" vsdconfigtool)

add_custom_command(TARGET cppwinrtvisualizer POST_BUILD
    COMMAND ${vsdconfigtool} ${CMAKE_CURRENT_SOURCE_DIR}/cppwinrtvisualizer.vsdconfigxml "${CMAKE_CURRENT_BINARY_DIR}/cppwinrtvisualizer.dll" "${cppwinrtvisualizer_vsdconfig}"
)

set_target_properties(cppwinrtvisualizer PROPERTIES "cppwinrt_natvis_dir" ${cppwinrt_natvis_dir})

endif()