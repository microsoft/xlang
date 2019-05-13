file(TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/cppwinrt.exe" cppwinrt_exe)
file(TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}" CmakeOutDir)

set(build msbuild /nologo /m /p:Configuration=${CMAKE_BUILD_TYPE},Platform=$ENV{VSCMD_ARG_TGT_ARCH},XlangBuildVersion=${XLANG_BUILD_VERSION},CmakeOutDir=${CmakeOutDir}  ${CMAKE_CURRENT_SOURCE_DIR}/cppwinrt.sln)

add_custom_command(OUTPUT ${cppwinrt_exe}
    COMMAND ${build} /t:cppwinrt
)

add_custom_target(cppwinrt ALL DEPENDS ${cppwinrt_exe})

add_custom_target(cppwinrt_test ALL DEPENDS cppwinrt 
    COMMAND ${cppwinrt_exe} -in sdk -out ${CMAKE_CURRENT_BINARY_DIR} -verbose
    COMMAND ${build} /t:fast_fwd
    COMMAND ${build} /t:test
    COMMAND ${build} /t:test_fast
    COMMAND ${build} /t:test_fast_fwd
    COMMAND ${build} /t:old_tests\\test_old
)

set_target_properties(cppwinrt PROPERTIES "cppwinrt_exe" ${cppwinrt_exe})

# For x86, build all arches of fast forwarder lib for NuGet dependency
if (WIN32 AND ("$ENV{VSCMD_ARG_TGT_ARCH}" STREQUAL "x86"))

file(TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/cppwinrt_fast_forward.lib" cppwinrt_fast_fwd_x86)

add_custom_command(OUTPUT ${cppwinrt_fast_fwd_x86}
    COMMAND ${build} /t:fast_fwd /p:Platform=x86
    COMMAND ${build} /t:fast_fwd /p:Platform=x64
    COMMAND ${build} /t:fast_fwd /p:Platform=arm
    COMMAND ${build} /t:fast_fwd /p:Platform=arm64
)

add_custom_target(cppwinrt_fast_fwd ALL DEPENDS ${cppwinrt_fast_fwd_x86})

set_target_properties(cppwinrt_fast_fwd PROPERTIES "cppwinrt_fast_fwd_x86" ${cppwinrt_fast_fwd_x86})

endif()
