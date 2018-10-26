# The natvis cppwinrtvisualizer.dll is only targeted at Visual Studio (on Windows)
if (WIN32)

cmake_minimum_required(VERSION 3.9)

project(cppwinrtvisualizer)
add_library(cppwinrtvisualizer SHARED)

target_sources(cppwinrtvisualizer PUBLIC pch.cpp dllmain.cpp cppwinrt_visualizer.cpp object_visualizer.cpp property_visualizer.cpp cppwinrtvisualizer.def)
TARGET_CONFIG_MSVC_PCH(cppwinrtvisualizer pch.cpp pch.h)
target_include_directories(cppwinrtvisualizer PUBLIC ${XLANG_LIBRARY_PATH} "$ENV{VSInstallDir}DIA SDK/include" "$ENV{VSInstallDir}VSSDK/VisualStudioIntegration/Common/inc")
# VSSDK headers not conformant
string(APPEND CMAKE_CXX_FLAGS " /wd4471")

target_link_libraries(cppwinrtvisualizer windowsapp shell32 advapi32 $ENV{VSInstallDir}VSSDK/VisualStudioIntegration/Common/lib/$ENV{VSCMD_ARG_TGT_ARCH}/vsdebugeng.lib)

endif()