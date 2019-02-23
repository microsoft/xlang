cmake_minimum_required(VERSION 3.9)

file(TO_NATIVE_PATH ${CMAKE_BINARY_DIR}/tool/cpp/cppwinrt/cppwinrt.exe cpp_exe)
file(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR} project_folder)

file(TO_NATIVE_PATH "${project_folder}/metadata" metadata_folder)
file(TO_NATIVE_PATH "${metadata_folder}/Windows.Foundation.FoundationContract.winmd" foundation_winmd)

file(TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/generated" generated_files)
file(TO_NATIVE_PATH "${generated_files}/component.winmd" component_winmd)
file(TO_NATIVE_PATH "${generated_files}/winrt/Windows.Foundation.h" foundation_h)
file(TO_NATIVE_PATH "${generated_files}/module.g.cpp" module_g_cpp)

set(sources
    pch.cpp
    ${module_g_cpp}
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

add_library(test_component STATIC "")
target_sources(test_component PRIVATE ${sources} )
target_include_directories(test_component BEFORE PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${generated_files})

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
    COMMAND midlrt ${project_folder}\\component.idl /nomidl /winrt /winmd ${component_winmd} /metadata_dir ${metadata_folder} /reference ${foundation_winmd} /h "nul"
    DEPENDS ${project_folder}\\component.idl ${generated_files}
)

# winrt/coroutine.h depends on Windows.System and Windows.UI.Core.h, so we have to generate the
# entire winrt projection here (aka -in ${metadata_folder}).

add_custom_command(OUTPUT ${foundation_h}
    COMMAND ${cpp_exe} -in ${metadata_folder} -out ${generated_files} -verbose
    DEPENDS ${component_winmd} ${generated_files} cppwinrt
)

add_custom_command(OUTPUT ${module_g_cpp}
    COMMAND ${cpp_exe} -in ${component_winmd} -ref ${foundation_winmd} -include Component -out ${generated_files} -component -verbose -prefix -base -lib test -opt
    DEPENDS ${foundation_h} ${generated_files} cppwinrt
)

add_custom_target(test_component_depends ALL DEPENDS ${module_g_cpp})
add_dependencies(test_component cppwinrt test_component_depends)