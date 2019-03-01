cmake_minimum_required(VERSION 3.9)

# The natvis cppwinrtvisualizer.dll is only targeted at Visual Studio (on Windows)
if (WIN32)

file(TO_NATIVE_PATH "${CMAKE_BINARY_DIR}/build_tools/nuget.exe" nuget_exe)
file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/$ENV{VSCMD_ARG_TGT_ARCH}/${CMAKE_BUILD_TYPE}" cppwinrt_natvis_dir)
file(TO_NATIVE_PATH ${cppwinrt_natvis_dir}/cppwinrtvisualizer.dll cppwinrtvisualizer_dll)

set(build_visualizer msbuild ${CMAKE_CURRENT_SOURCE_DIR}/cppwinrtvisualizer.vcxproj /nologo /m /p:Configuration=${CMAKE_BUILD_TYPE})

file(DOWNLOAD https://dist.nuget.org/win-x86-commandline/latest/nuget.exe ${nuget_exe})

add_custom_command(OUTPUT ${cppwinrtvisualizer_dll}
    COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR} & ${nuget_exe} restore
    COMMAND ${build_visualizer} /p:Platform=x86
    COMMAND ${build_visualizer} /p:Platform=x64
)

add_custom_target(cppwinrtvisualizer ALL DEPENDS ${cppwinrtvisualizer_dll})

set_target_properties(cppwinrtvisualizer PROPERTIES "cppwinrt_natvis_dir" ${cppwinrt_natvis_dir})

endif()