cmake_minimum_required(VERSION 3.9)

add_executable(cpp_test "")
target_include_directories(cpp_test BEFORE PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/implementation "${CMAKE_CURRENT_SOURCE_DIR}/generated" ${XLANG_LIBRARY_PATH} ${XLANG_TEST_INC_PATH})
target_sources(cpp_test PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/generated/module.g.cpp"
    pch.cpp
    activation.cpp
    test/AbiGuard.cpp
    test/AgileRef.cpp
    test/Async.cpp
    test/AsyncNoSuspend.cpp
    test/AsyncReturn.cpp
    test/CmdReader.cpp
    test/Collections.cpp
    test/Composable.cpp 
    test/CustomError.cpp
    test/Edge.cpp
    test/Enum.cpp
    test/Events.cpp
    test/EventsInvalid.cpp
    test/Fast.cpp
    test/FinalRelease.cpp
    test/Names.cpp
    test/Structs.cpp
    test/VariadicDelegate.cpp
)
target_link_libraries(cpp_test test_component)

if (MSVC)
    TARGET_CONFIG_MSVC_PCH(cpp_test pch.cpp pch.h)
    target_link_libraries(cpp_test windowsapp)
else()
    target_link_libraries(cpp_test c++ c++abi c++experimental)
    target_link_libraries(cpp_test -lpthread)
endif()

if (CMAKE_CXX_COMPILER MATCHES "clang-cl")
    target_sources(cpp_test PUBLIC main.cpp)
else()
    target_sources(cpp_test PUBLIC
        main.cpp
        test/Agility.cpp
        test/Interop.cpp
        test/Capture.cpp
        test/AsyncAutoCancel.cpp
        test/AsyncCancelCallback.cpp
        test/AsyncCheckCancel.cpp
        test/AsyncLocal.cpp
        test/AsyncProgress.cpp
        test/AsyncResult.cpp
        test/AsyncSuspend.cpp
        test/AsyncThrow.cpp
    )
endif()

file(TO_NATIVE_PATH ${CMAKE_CURRENT_BINARY_DIR}/../../tool/cpp/cppwinrt/cppwinrt.exe cpp_exe)
file(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR} project_folder)
file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/generated" generated_files)
file(TO_NATIVE_PATH "${generated_files}/component.winmd" component_winmd)
file(TO_NATIVE_PATH "${project_folder}/generated/winrt/Windows.Foundation.h" foundation_h)
file(TO_NATIVE_PATH "${generated_files}/module.g.cpp" mogule_g_cpp)

add_custom_command(OUTPUT ${generated_files}
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${generated_files}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${generated_files}
    DEPENDS ${project_folder}\\component.idl
)

add_custom_command(OUTPUT ${component_winmd}
    COMMAND midlrt ${project_folder}\\component.idl /nomidl /winrt /winmd ${component_winmd} /metadata_dir ${project_folder}\\metadata /reference ${project_folder}\\metadata\\Windows.Foundation.winmd /h "nul"
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

add_custom_target(cpp_test_depends ALL DEPENDS ${mogule_g_cpp})
add_dependencies(cpp_test cppwinrt cpp_test_depends)

install(TARGETS cpp_test DESTINATION "test")
if (WIN32)
    install(FILES $<TARGET_PDB_FILE:cpp_test> DESTINATION "test" OPTIONAL)
endif ()