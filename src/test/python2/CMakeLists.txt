cmake_minimum_required(VERSION 3.9)

set(python_module_name pytestcmp)

find_file(orig_cppwinrt_exe cppwinrt.exe)
file(TO_NATIVE_PATH "${CMAKE_BINARY_DIR}/tool/python/pywinrt.exe" pywinrt_exe)
file(TO_NATIVE_PATH "${CMAKE_BINARY_DIR}/test/component/attributes.winmd" attributes_winmd)
file(TO_NATIVE_PATH "${CMAKE_BINARY_DIR}/test/component/component.winmd" component_winmd)
file(TO_NATIVE_PATH "${CMAKE_SOURCE_DIR}/test/component/Windows.Foundation.FoundationContract.winmd" foundation_winmd)

file(TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/generated/cpp" generated_cppwinrt_files)
file(TO_NATIVE_PATH "${generated_cppwinrt_files}/winrt/base.h" base_h)
file(TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/generated/py" generated_pywinrt_files)
file(TO_NATIVE_PATH "${generated_pywinrt_files}/setup.py" setup_py)
file(TO_NATIVE_PATH "${generated_pywinrt_files}/_${python_module_name}.cp37-win_amd64.pyd" module_pyd)

add_custom_command(OUTPUT ${base_h}
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${generated_cppwinrt_files}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${generated_cppwinrt_files}
    COMMAND ${orig_cppwinrt_exe} -in ${attributes_winmd} -in ${component_winmd} -in ${foundation_winmd} -out ${generated_cppwinrt_files} -verbose
    DEPENDS ${component_winmd}
)

add_custom_command(OUTPUT ${setup_py}
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${generated_pywinrt_files}
    COMMAND ${pywinrt_exe} -input ${component_winmd} -input ${attributes_winmd} -input ${foundation_winmd} -output ${generated_pywinrt_files} -module ${python_module_name}
    DEPENDS pywinrt test_component
)

add_custom_target(python_test_2 DEPENDS ${base_h} ${setup_py})

# add_custom_command(OUTPUT ${module_pyd} 
#   WORKING_DIRECTORY ${generated_pywinrt_files}
#   COMMAND py setup.py build_ext --inplace
#   DEPENDS python_test_2
# )

# add_custom_target(build_python_test_2 DEPENDS ${module_pyd})