cmake_minimum_required(VERSION 3.9)

execute_process(
    COMMAND py -c "import sys; vi = sys.version_info; print('cp{0}{1}-{2}'.format(vi.major, vi.minor, 'win_amd64' if sys.maxsize > 2**32 else 'win32'))"
    OUTPUT_VARIABLE python_version_info
)

set(python_module_name pyrt)
file(TO_NATIVE_PATH ${CMAKE_CURRENT_BINARY_DIR}/../../tool/python/pywinrt.exe pywinrt_exe)
file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/generated" generated_files)
file(TO_NATIVE_PATH "$ENV{WindowsSdkDir}References/$ENV{WindowsSDKVersion}" winsdk_references_root)
file(TO_NATIVE_PATH "${generated_files}/build/_${python_module_name}.${python_version_info}" module_path_no_ext)
file(TO_NATIVE_PATH "${module_path_no_ext}.pyd" module_path)
file(TO_NATIVE_PATH "${module_path_no_ext}.pdb" module_pdb_path)
file(TO_NATIVE_PATH "${generated_files}/setup.py" setup_py)

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

    # uncomment out the following to run complete_winml_tutorial.py
    # Windows.AI.MachineLearning
    # Windows.Graphics.Imaging
    # Windows.Media
    # Windows.Storage
)

set(excluded_namespaces
    Windows.Foundation.Diagnostics.
    Windows.Foundation.Metadata.
    Windows.Media.Audio.
    Windows.Media.Capture.
    Windows.Media.Casting.
    Windows.Media.ClosedCaptioning.
    Windows.Media.ContentRestrictions.
    Windows.Media.Control.
    Windows.Media.Core.
    Windows.Media.Devices.
    Windows.Media.DialProtocol.
    Windows.Media.Editing.
    Windows.Media.Effects.
    Windows.Media.FaceAnalysis.
    Windows.Media.Import.
    Windows.Media.MediaProperties.
    Windows.Media.Ocr.
    Windows.Media.Playback.
    Windows.Media.Playlists.
    Windows.Media.PlayTo.
    Windows.Media.Protection.
    Windows.Media.Render.
    Windows.Media.SpeechRecognition.
    Windows.Media.SpeechSynthesis.
    Windows.Media.Streaming.
    Windows.Media.Transcoding.
    Windows.Storage.AccessCache.
    Windows.Storage.BulkAccess.
    Windows.Storage.Compression.
    Windows.Storage.FileProperties.
    Windows.Storage.Pickers.
    Windows.Storage.Provider.
    # Windows.Storage.Search.
    # Windows.Storage.Streams.
)

set(pywinrt_cmdline "-output ${generated_files} -module ${python_module_name}")

foreach (winmd ${winmd_files})
    file(TO_NATIVE_PATH "${winsdk_references_root}/${winmd}" contract_root)
    file(GLOB_RECURSE winmd_path "${contract_root}/${winmd}.winmd")
    list(LENGTH winmd_path winmd_path_length)
    if(winmd_path_length EQUAL 1)
        string(APPEND pywinrt_cmdline " -input \"${winmd_path}\"")
    endif()
endforeach()

foreach(ns ${included_namespaces})
    string(APPEND pywinrt_cmdline " -include ${ns}")
endforeach()

foreach(ns ${excluded_namespaces})
    string(APPEND pywinrt_cmdline " -exclude ${ns}")
endforeach()

add_custom_command(
  OUTPUT ${setup_py} 
  DEPENDS ${pywinrt_exe}
  COMMAND ${pywinrt_exe} ${pywinrt_cmdline}
)

add_custom_command(
  OUTPUT ${module_path} 
  BYPRODUCTS ${module_pdb_path}
  DEPENDS ${setup_py}
  WORKING_DIRECTORY ${generated_files}
  COMMAND py setup.py develop
)

add_custom_target(generate_python_test ALL DEPENDS ${setup_py})
add_custom_target(python_test ALL DEPENDS ${module_path})

# TODO: include python_test in ALL once issue running python on build system is resolved 
set_target_properties(python_test PROPERTIES EXCLUDE_FROM_ALL TRUE)
