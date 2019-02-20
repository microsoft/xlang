cmake_minimum_required(VERSION 3.9)

# The natvis cppwinrtvisualizer.dll is only targeted at Visual Studio (on Windows)
if (WIN32 AND vssdk_installed)

file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${XLANG_BUILD_ARCH}/${CMAKE_BUILD_TYPE}" cppwinrt_natvis_dir)
file(TO_NATIVE_PATH ${cppwinrt_natvis_dir}/cppwinrtvisualizer.dll cppwinrtvisualizer_dll)

set(build_visualizer msbuild ${CMAKE_CURRENT_SOURCE_DIR}/cppwinrtvisualizer.vcxproj /nologo /m /p:Configuration=${CMAKE_BUILD_TYPE},Platform=${XLANG_BUILD_ARCH})

add_custom_command(OUTPUT ${cppwinrtvisualizer_dll}
    COMMAND ${build_visualizer}
)

add_custom_target(cppwinrtvisualizer ALL DEPENDS ${cppwinrtvisualizer_dll})

set_target_properties(cppwinrtvisualizer PROPERTIES "cppwinrt_natvis_dir" ${cppwinrt_natvis_dir})

endif()