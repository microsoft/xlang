cmake_minimum_required(VERSION 3.9)

set(pywinrt_exe ${CMAKE_CURRENT_BINARY_DIR}/../../tool/python/pywinrt.exe)
get_filename_component(output_dir ${CMAKE_CURRENT_SOURCE_DIR}/output REALPATH)

file(TO_NATIVE_PATH "$ENV{WindowsSdkDir}References/$ENV{WindowsSDKVersion}" REFERENCES_ROOT)

set(winmd_files
    Windows.Foundation.FoundationContract
    Windows.Foundation.UniversalApiContract
    Windows.AI.MachineLearning.MachineLearningContract
)

set(included_namespaces
    Windows.Data.Json
    Windows.Devices.Geolocation
    Windows.Foundation
    Windows.Graphics.DirectX

    Windows.AI.MachineLearning
    Windows.Graphics.Imaging
    Windows.Media
    Windows.Storage    
)

set(excluded_namespaces
    Windows.Foundation.Diagnostics
    Windows.Foundation.Metadata
    Windows.Media.Audio
    Windows.Media.Capture
    Windows.Media.Casting
    Windows.Media.ClosedCaptioning
    Windows.Media.ContentRestrictions
    Windows.Media.Control
    Windows.Media.Core
    Windows.Media.Devices
    Windows.Media.DialProtocol
    Windows.Media.Editing
    Windows.Media.Effects
    Windows.Media.FaceAnalysis
    Windows.Media.Import
    Windows.Media.MediaProperties
    Windows.Media.Ocr
    Windows.Media.Playback
    Windows.Media.Playlists
    Windows.Media.PlayTo
    Windows.Media.Protection
    Windows.Media.Render
    Windows.Media.SpeechRecognition
    Windows.Media.SpeechSynthesis
    Windows.Media.Streaming
    Windows.Media.Transcoding
    Windows.Storage.AccessCache
    Windows.Storage.BulkAccess
    Windows.Storage.Compression
    Windows.Storage.FileProperties
    Windows.Storage.Pickers
    Windows.Storage.Provider
    # Windows.Storage.Search
    # Windows.Storage.Streams
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
set(setup_py ${output_dir}/setup.py)

add_custom_command(
  OUTPUT ${setup_py} 
  DEPENDS ${pywinrt_exe}
  COMMAND ${pywinrt_exe} ${cmdline}
)

add_custom_target(test_python ALL DEPENDS ${setup_py})
add_dependencies(test_python pywinrt)
