cmake_minimum_required(VERSION 3.9)

file(TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/generated" generated_files)
file(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR} project_folder)
file(TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/attributes.winmd" attributes_winmd)
file(TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/component.winmd" component_winmd)
file(TO_NATIVE_PATH "${project_folder}/Windows.Foundation.FoundationContract.winmd" foundation_winmd)
file(TO_NATIVE_PATH ${CMAKE_CURRENT_BINARY_DIR}/../../tool/cpp/cppwinrt/cppwinrt.exe cpp_exe)
file(TO_NATIVE_PATH "${generated_files}/module.g.cpp" module_g_cpp)

set(foundation_generated_namespaces
    Windows.Foundation
    Windows.Foundation.Collections
    Windows.Foundation.Metadata
)

set(foundation_generated_files "${generated_files}\\winrt\\base.h")
foreach(generated_ns ${foundation_generated_namespaces})
    file(TO_NATIVE_PATH "${generated_files}\\winrt\\${generated_ns}.h" native_generated_file)
    list(APPEND foundation_generated_files ${native_generated_file})

    foreach(impl_file_num 0 1 2)
        file(TO_NATIVE_PATH "${generated_files}\\winrt\\impl\\${generated_ns}.${impl_file_num}.h" native_generated_impl_file)
        list(APPEND foundation_generated_files ${native_generated_impl_file})
    endforeach()
endforeach()

# set(component_generated_namespaces
#     Component.Async
#     Component.Collections
#     Component.Composable
#     Component.Edge
#     Component.Events
#     Component.Fast
#     Component
#     Component.Params
#     Component.Result
#     Component.Structs
# )

# set(component_generated_classes
#     Component.Async.Class
#     Component.Collections.Class
#     Component.Composable.Base
#     Component.Composable.Derived
#     Component.Edge.EmptyClass
#     Component.Edge.OneClass
#     Component.Edge.StaticClass
#     Component.Edge.ThreeClass
#     Component.Edge.TwoClass
#     Component.Edge.ZeroClass
#     Component.Events.Class
#     Component.Fast.FastClass
#     Component.Fast.SlowClass
#     Component.Params.Class
#     Component.Result.Class
# )
# Component.Async.Class.g.cpp
# Component.Async.Class.g.h
# Component.Collections.Class.g.cpp
# Component.Collections.Class.g.h
# Component.Composable.Base.g.cpp
# Component.Composable.Base.g.h
# Component.Composable.Derived.g.cpp
# Component.Composable.Derived.g.h
# Component.Edge.EmptyClass.g.cpp
# Component.Edge.EmptyClass.g.h
# Component.Edge.OneClass.g.cpp
# Component.Edge.OneClass.g.h
# Component.Edge.StaticClass.g.cpp
# Component.Edge.StaticClass.g.h
# Component.Edge.ThreeClass.g.cpp
# Component.Edge.ThreeClass.g.h
# Component.Edge.TwoClass.g.cpp
# Component.Edge.TwoClass.g.h
# Component.Edge.ZeroClass.g.cpp
# Component.Edge.ZeroClass.g.h
# Component.Events.Class.g.cpp
# Component.Events.Class.g.h
# Component.Fast.FastClass.g.cpp
# Component.Fast.FastClass.g.h
# Component.Fast.SlowClass.g.cpp
# Component.Fast.SlowClass.g.h
# Component.Params.Class.g.cpp
# Component.Params.Class.g.h
# Component.Result.Class.g.cpp
# Component.Result.Class.g.h
# winrt\Component.Async.h
# winrt\Component.Collections.h
# winrt\Component.Composable.h
# winrt\Component.Edge.h
# winrt\Component.Events.h
# winrt\Component.Fast.h
# winrt\Component.h
# winrt\Component.Params.h
# winrt\Component.Result.h
# winrt\Component.Structs.h
# winrt\impl\Component.0.h
# winrt\impl\Component.1.h
# winrt\impl\Component.2.h
# winrt\impl\Component.Async.0.h
# winrt\impl\Component.Async.1.h
# winrt\impl\Component.Async.2.h
# winrt\impl\Component.Collections.0.h
# winrt\impl\Component.Collections.1.h
# winrt\impl\Component.Collections.2.h
# winrt\impl\Component.Composable.0.h
# winrt\impl\Component.Composable.1.h
# winrt\impl\Component.Composable.2.h
# winrt\impl\Component.Edge.0.h
# winrt\impl\Component.Edge.1.h
# winrt\impl\Component.Edge.2.h
# winrt\impl\Component.Events.0.h
# winrt\impl\Component.Events.1.h
# winrt\impl\Component.Events.2.h
# winrt\impl\Component.Fast.0.h
# winrt\impl\Component.Fast.1.h
# winrt\impl\Component.Fast.2.h
# winrt\impl\Component.Params.0.h
# winrt\impl\Component.Params.1.h
# winrt\impl\Component.Params.2.h
# winrt\impl\Component.Result.0.h
# winrt\impl\Component.Result.1.h
# winrt\impl\Component.Result.2.h
# winrt\impl\Component.Structs.0.h
# winrt\impl\Component.Structs.1.h
# winrt\impl\Component.Structs.2.h
# )

add_library(test_component SHARED)
target_include_directories(test_component BEFORE PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${generated_files})
target_sources(test_component PUBLIC
    pch.cpp
    "${generated_files}/module.g.cpp"
    Component.Async.Class.cpp
    Component.Collections.Class.cpp
    Component.Composable.Base.cpp
    Component.Composable.Derived.cpp
    Component.Edge.EmptyClass.cpp
    Component.Edge.OneClass.cpp
    Component.Edge.StaticClass.cpp
    Component.Edge.ThreeClass.cpp
    Component.Edge.TwoClass.cpp
    Component.Edge.ZeroClass.cpp
    Component.Events.Class.cpp
    Component.Fast.FastClass.cpp
    Component.Fast.SlowClass.cpp
    Component.Params.Class.cpp
    Component.Result.Class.cpp
)

if (MSVC)
    TARGET_CONFIG_MSVC_PCH(test_component pch.cpp pch.h)
    target_link_libraries(test_component windowsapp)
else()
    target_link_libraries(test_component c++ c++abi c++experimental)
    target_link_libraries(test_component -lpthread)
endif()


add_custom_command(OUTPUT ${attributes_winmd}
    COMMAND midlrt ${project_folder}\\attributes.idl /nomidl /winrt /winmd ${attributes_winmd} /metadata_dir ${project_folder} /reference ${foundation_winmd} /h "nul"
    DEPENDS ${project_folder}\\attributes.idl 
)

add_custom_command(OUTPUT ${component_winmd}
    COMMAND midlrt ${project_folder}\\component.idl /nomidl /winrt /winmd ${component_winmd} /metadata_dir ${project_folder} /reference ${foundation_winmd} /reference ${attributes_winmd} /h "nul"
    DEPENDS ${project_folder}\\component.idl ${attributes_winmd} 
)

add_custom_command(OUTPUT ${foundation_generated_files}
    COMMAND ${cpp_exe} -in ${foundation_winmd} -out ${generated_files} -verbose
    DEPENDS ${component_winmd} cppwinrt
)

add_custom_command(OUTPUT ${module_g_cpp}
    COMMAND ${cpp_exe} -in ${component_winmd} -ref ${foundation_winmd} ${attributes_winmd} -out ${generated_files} -component -verbose -prefix -base -lib test -opt
    DEPENDS ${component_winmd} cppwinrt
)

add_custom_target(test_component_depends ALL DEPENDS ${component_winmd} ${module_g_cpp})
# ${foundation_h} ${module_g_cpp})
add_dependencies(test_component test_component_depends)
