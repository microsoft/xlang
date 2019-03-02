cmake_minimum_required(VERSION 3.9)

file(TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/cppwinrt.exe" cppwinrt_exe)
file(TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}" CmakeOutDir)

set(build msbuild /nologo /m /p:Configuration=${CMAKE_BUILD_TYPE},Platform=$ENV{VSCMD_ARG_TGT_ARCH},CppWinRTVersion=${XLANG_BUILD_VERSION},CmakeOutDir=${CmakeOutDir}  ${CMAKE_CURRENT_SOURCE_DIR}/cppwinrt.sln)

add_custom_command(OUTPUT ${cppwinrt_exe}
    COMMAND ${build} /t:cppwinrt
)

add_custom_target(cppwinrt ALL DEPENDS ${cppwinrt_exe})

add_custom_target(cppwinrt_test ALL DEPENDS cppwinrt
    COMMAND ${cppwinrt_exe} -in sdk -out ${CMAKE_CURRENT_BINARY_DIR} -verbose
    COMMAND ${build} /t:test_cpp
    COMMAND ${build} /t:old_tests\\Tests
)
