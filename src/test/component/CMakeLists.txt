cmake_minimum_required(VERSION 3.9)

file(TO_NATIVE_PATH ${CMAKE_BINARY_DIR}/tool/cpp/cppwinrt/cppwinrt.exe cpp_exe)
file(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR} project_folder)
# file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/generated" generated_files)
file(TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/generated" generated_files)
file(TO_NATIVE_PATH "${generated_files}/component.winmd" component_winmd)
file(TO_NATIVE_PATH "${generated_files}/winrt/Windows.Foundation.h" foundation_h)
file(TO_NATIVE_PATH "${generated_files}/module.g.cpp" mogule_g_cpp)

add_library(test_component MODULE "")
set_target_properties(test_component PROPERTIES OUTPUT_NAME "component")
target_include_directories(test_component BEFORE PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR} 
    ${generated_files}
)

target_sources(test_component PUBLIC
    ${mogule_g_cpp}
    pch.cpp
    module.def
    implementation/Component.Async.Class.cpp
    implementation/Component.Collections.Class.cpp
    implementation/Component.Composable.Base.cpp
    implementation/Component.Composable.Derived.cpp
    implementation/Component.Edge.EmptyClass.cpp
    implementation/Component.Edge.OneClass.cpp
    implementation/Component.Edge.StaticClass.cpp
    implementation/Component.Edge.ThreeClass.cpp
    implementation/Component.Edge.TwoClass.cpp
    implementation/Component.Edge.ZeroClass.cpp
    implementation/Component.Events.Class.cpp
    implementation/Component.Fast.FastClass.cpp
    implementation/Component.Fast.SlowClass.cpp
    implementation/Component.Params.Class.cpp
    implementation/Component.Result.Class.cpp
)

# file(TO_NATIVE_PATH ${CMAKE_CURRENT_BINARY_DIR}/../../tool/cpp/cppwinrt/cppwinrt.exe cpp_exe)
# file(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR} project_folder)
# file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/generated" generated_files)

# file(TO_NATIVE_PATH ${CMAKE_BINARY_DIR}/tool/cpp/cppwinrt/cppwinrt.exe cpp_exe)
# file(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR} project_folder)
# file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/generated" generated_files)
# file(TO_NATIVE_PATH "${generated_files}/component.winmd" component_winmd)
# file(TO_NATIVE_PATH "${project_folder}/Windows.Foundation.FoundationContract.winmd" foundation_winmd)
# file(TO_NATIVE_PATH "${generated_files}/winrt/Windows.Foundation.h" foundation_h)
# file(TO_NATIVE_PATH "${generated_files}/module.g.cpp" module_g_cpp)

# file(TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/component.winmd" component_winmd)
# file(TO_NATIVE_PATH "${project_folder}/Windows.Foundation.FoundationContract.winmd" foundation_winmd)
# file(TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/generated" generated_files)

#add_library(test_component SHARED "")
#target_include_directories(test_component BEFORE PUBLIC ${generated_files}) 
#${CMAKE_CURRENT_SOURCE_DIR}) 
#    ${generated_files} 
#
# # target_compile_options(test_component PUBLIC /X)
# target_include_directories(test_component BEFORE PUBLIC 
#     ${CMAKE_CURRENT_SOURCE_DIR} 
#     ${XLANG_LIBRARY_PATH})

# target_sources(test_component PUBLIC
#     pch.cpp
#     ${module_g_cpp}
#     module.def
#     Component.Async.Class.cpp
#     Component.Collections.Class.cpp
#     Component.Composable.Base.cpp
#     Component.Composable.Derived.cpp
#     Component.Edge.EmptyClass.cpp
#     Component.Edge.OneClass.cpp
#     Component.Edge.StaticClass.cpp
#     Component.Edge.ThreeClass.cpp
#     Component.Edge.TwoClass.cpp
#     Component.Edge.ZeroClass.cpp
#     Component.Events.Class.cpp
#     Component.Fast.FastClass.cpp
#     Component.Fast.SlowClass.cpp
#     Component.Params.Class.cpp
#     Component.Result.Class.cpp
# )

if (MSVC)
    TARGET_CONFIG_MSVC_PCH(test_component pch.cpp pch.h)
    target_link_libraries(test_component windowsapp)
else()
    target_link_libraries(test_component c++ c++abi c++experimental)
    target_link_libraries(test_component -lpthread)
endif()

add_custom_command(OUTPUT ${generated_files}
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${generated_files}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${generated_files}
    DEPENDS ${project_folder}\\component.idl
)

add_custom_command(OUTPUT ${component_winmd}
    COMMAND midlrt ${project_folder}\\component.idl /nomidl /winrt /winmd ${component_winmd} /metadata_dir ${project_folder} /reference ${project_folder}\\Windows.Foundation.FoundationContract.winmd /h "nul"
    DEPENDS ${project_folder}\\component.idl ${generated_files}
)

add_custom_command(OUTPUT ${foundation_h}
    COMMAND ${cpp_exe} -in local -out ${generated_files} -verbose
    DEPENDS ${component_winmd} ${generated_files} cppwinrt
)

add_custom_command(OUTPUT ${mogule_g_cpp}
    COMMAND ${cpp_exe} -in ${component_winmd} -ref local -include Component -out ${generated_files} -component -verbose -prefix -base -lib test -opt
    DEPENDS ${foundation_h} ${generated_files} cppwinrt
)

add_custom_target(test_component_depends ALL DEPENDS ${mogule_g_cpp})
add_dependencies(test_component cppwinrt test_component_depends)
