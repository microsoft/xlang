cmake_minimum_required(VERSION 3.9)

file(TO_NATIVE_PATH ${CMAKE_BINARY_DIR}/tool/cpp/cppwinrt/cppwinrt.exe cpp_exe)
file(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR} project_folder)
file(TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/attributes.winmd" attributes_winmd)
file(TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/component.winmd" component_winmd)
file(TO_NATIVE_PATH "${project_folder}/Windows.Foundation.FoundationContract.winmd" foundation_winmd)
file(TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/generated" generated_files)
file(TO_NATIVE_PATH "${generated_files}/winrt/Windows.Foundation.h" foundation_h)
file(TO_NATIVE_PATH "${generated_files}/module.g.cpp" module_g_cpp)

add_library(test_component SHARED)
target_include_directories(test_component BEFORE PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${generated_files})
target_sources(test_component PUBLIC
    pch.cpp
    "${generated_files}/module.g.cpp"
    module.def
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

add_custom_command(OUTPUT ${foundation_h}
    COMMAND ${cpp_exe} -in ${foundation_winmd} -out ${generated_files} -verbose
    DEPENDS cppwinrt
)

add_custom_command(OUTPUT ${module_g_cpp}
    COMMAND ${cpp_exe} -in ${component_winmd} -ref ${foundation_winmd} ${attributes_winmd} -out ${generated_files} -component -verbose -prefix -base -opt
    DEPENDS ${component_winmd} cppwinrt
)

add_custom_target(test_component_depends ALL DEPENDS ${component_winmd} ${foundation_h} ${module_g_cpp} )
add_dependencies(test_component test_component_depends)
