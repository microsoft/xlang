cmake_minimum_required(VERSION 3.9)

set(pywinrt_exe ${CMAKE_CURRENT_BINARY_DIR}/../../tool/python/pywinrt.exe)
get_filename_component(output_dir ${CMAKE_CURRENT_SOURCE_DIR}/output REALPATH)

file(TO_NATIVE_PATH "$ENV{WindowsSdkDir}References/$ENV{WindowsSDKVersion}" REFERENCES_ROOT)

set(winmd_files
    Windows.Foundation.FoundationContract
    Windows.Devices.DevicesLowLevelContract
    Windows.Foundation.UniversalApiContract
    Windows.AI.MachineLearning.MachineLearningContract
)

set(included_namespaces
    Windows.Data.Json
    Windows.Devices.Geolocation
    Windows.Foundation
    Windows.Graphics.DirectX
)

set(excluded_namespaces
    Windows.Foundation.Deferral
    Windows.Foundation.Diagnostics
    Windows.Foundation.IPropertyValue
    Windows.Foundation.IRef
    Windows.Foundation.Metadata
    Windows.Foundation.PropertyValue
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
set(pybase_h ${output_dir}/pybase.h)

message(${pybase_h})
add_custom_command(
  OUTPUT ${pybase_h} 
  DEPENDS ${pywinrt_exe}
  COMMAND ${pywinrt_exe} ${cmdline}
)

add_custom_target(test_python ALL DEPENDS ${pybase_h})
add_dependencies(test_python pywinrt)
