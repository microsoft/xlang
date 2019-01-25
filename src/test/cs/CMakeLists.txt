cmake_minimum_required(VERSION 3.9)


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

file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/generated" generated_files_folder)
set(cswinrt_cmdline "-output ${generated_files_folder}")

file(TO_NATIVE_PATH "C:/Program Files (x86)/Windows Kits/10" WindowsSdkDir)
set(WindowsSDKVersion "10.0.17763.0")

file(TO_NATIVE_PATH "$ENV{WindowsSdkDir}References/$ENV{WindowsSDKVersion}" winsdk_references_root)
foreach (winmd ${winmd_files})
    file(TO_NATIVE_PATH "${winsdk_references_root}/${winmd}" CONTRACT_ROOT)
    file(GLOB_RECURSE winmd_path "${CONTRACT_ROOT}/${winmd}.winmd")
    list(LENGTH winmd_path winmd_path_length)
    if(winmd_path_length EQUAL 1)
        string(APPEND cswinrt_cmdline " -input \"${winmd_path}\"")
    endif()
endforeach()

foreach(ns ${included_namespaces})
    string(APPEND cswinrt_cmdline " -include ${ns}")
endforeach()

foreach(ns ${excluded_namespaces})
    string(APPEND cswinrt_cmdline " -exclude ${ns}")
endforeach()

file(TO_NATIVE_PATH "${generated_files_folder}/timestamp.txt" timestamp_txt)
file(TO_NATIVE_PATH ${CMAKE_BINARY_DIR}/tool/cs/Debug/cswinrt.exe cswinrt_exe)
add_custom_command(
  OUTPUT ${timestamp_txt}
  DEPENDS ${cswinrt_exe}
  COMMAND ${cswinrt_exe} ${cswinrt_cmdline}
)

add_custom_target(test_cs ALL DEPENDS ${timestamp_txt})
add_dependencies(test_cs cswinrt)
