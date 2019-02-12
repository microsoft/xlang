file(TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/../component/generated" generated_files)
file(TO_NATIVE_PATH "${generated_files}/module.g.cpp" module_g_cpp)

add_executable(cpp_test)
target_include_directories(cpp_test BEFORE PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/implementation "${CMAKE_CURRENT_SOURCE_DIR}/generated" ${XLANG_LIBRARY_PATH} ${XLANG_TEST_INC_PATH})
target_sources(cpp_test PUBLIC
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

install(TARGETS cpp_test DESTINATION "test")
if (WIN32)
    install(FILES $<TARGET_PDB_FILE:cpp_test> DESTINATION "test" OPTIONAL)
endif ()
