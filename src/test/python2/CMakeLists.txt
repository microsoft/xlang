cmake_minimum_required(VERSION 3.9)

file(TO_NATIVE_PATH "${CMAKE_BINARY_DIR}/tool/python/pywinrt.exe" pywinrt_exe)
file(TO_NATIVE_PATH "${CMAKE_BINARY_DIR}/test/component" test_component_bin_files)
file(TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/generated" generated_files)
file(TO_NATIVE_PATH "${generated_files}/setup.py" setup_py)
file(TO_NATIVE_PATH "${test_component_bin_files}/attributes.winmd" attributes_winmd)
file(TO_NATIVE_PATH "${test_component_bin_files}/component.winmd" component_winmd)
file(TO_NATIVE_PATH "${CMAKE_SOURCE_DIR}/test/component/Windows.Foundation.FoundationContract.winmd" foundation_winmd)

set(python_module_name pyrt2)

add_custom_command(OUTPUT ${setup_py}
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${generated_files}
    COMMAND ${pywinrt_exe} -input ${component_winmd} -input ${attributes_winmd} -input ${foundation_winmd} -output ${generated_files} -module ${python_module_name}
    DEPENDS pywinrt test_component
)

add_custom_target(python_test_2 DEPENDS ${setup_py})
