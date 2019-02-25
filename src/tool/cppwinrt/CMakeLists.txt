cmake_minimum_required(VERSION 3.9)

set(build msbuild /nologo /m /p:Configuration=${CMAKE_BUILD_TYPE},Platform=$ENV{VSCMD_ARG_TGT_ARCH} /p:CppWinRTVersion=${XLANG_BUILD_VERSION} ${CMAKE_CURRENT_SOURCE_DIR}/cppwinrt.sln)

file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/build/cppwinrt.exe" cppwinrt_exe)

add_custom_command(OUTPUT ${cppwinrt_exe}
    COMMAND ${build} /t:prebuild
    COMMAND ${build} /t:cppwinrt
)

add_custom_target(cppwinrt ALL DEPENDS ${cppwinrt_exe})

add_custom_target(cppwinrt_test ALL DEPENDS cppwinrt
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/build/cppwinrt.exe -in local -out ${CMAKE_CURRENT_SOURCE_DIR}/build -verbose
    COMMAND ${build} /t:test_component
    COMMAND ${build} /t:test_cpp
    COMMAND ${build} /t:old_tests\\Composable
    COMMAND ${build} /t:old_tests\\Component
    COMMAND ${build} /t:old_tests\\Tests
)
