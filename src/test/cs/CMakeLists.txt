cmake_minimum_required(VERSION 3.9)

set(cswinrt_exe ${CMAKE_CURRENT_BINARY_DIR}/../../tool/cs/cswinrt.exe)
get_filename_component(output_dir ${CMAKE_CURRENT_SOURCE_DIR}/output REALPATH)

file(TO_NATIVE_PATH "$ENV{WindowsSdkDir}References/$ENV{WindowsSDKVersion}" REFERENCES_ROOT)

set(winmd_files
    Windows.Foundation.FoundationContract
    Windows.Foundation.UniversalApiContract
)

set(included_namespaces
    Windows.Data.Json
    Windows.Devices.Geolocation
    Windows.Foundation
    Windows.Graphics.DirectX
)

set(excluded_namespaces
    Windows.Foundation.Diagnostics
    Windows.Foundation.Metadata
)

set(cmdline "-output ${output_dir}")

foreach (winmd ${winmd_files})
    file(TO_NATIVE_PATH "${REFERENCES_ROOT}/${winmd}" CONTRACT_ROOT)
    file(GLOB_RECURSE winmd_path "${CONTRACT_ROOT}/${winmd}.winmd")
    list(LENGTH winmd_path winmd_path_length)
    if(winmd_path_length EQUAL 1)
        string(APPEND cmdline " -input \"${winmd_path}\"")
    endif()
endforeach()

foreach(ns ${included_namespaces})
    string(APPEND cmdline " -include ${ns}")
endforeach()

foreach(ns ${excluded_namespaces})
    string(APPEND cmdline " -exclude ${ns}")
endforeach()

get_filename_component(output_dir ${CMAKE_CURRENT_SOURCE_DIR}/output REALPATH)
set(timestamp ${output_dir}/timestamp.txt)

add_custom_command(
  OUTPUT ${timestamp}
  DEPENDS ${cswinrt_exe}
  COMMAND ${cswinrt_exe} ${cmdline}
)

add_custom_target(test_cs ALL DEPENDS ${timestamp})
add_dependencies(test_cs cswinrt)
