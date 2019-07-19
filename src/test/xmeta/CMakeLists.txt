cmake_minimum_required(VERSION 3.9)

project(test_xmeta)

# using /MD flag for antlr4_runtime (for Visual C++ compilers only)
set(ANTLR4_WITH_STATIC_CRT ON)

 add_executable(test_xmeta "")

if (EXISTS "${CMAKE_SOURCE_DIR}/test/xmeta/example/midl3test")
    add_custom_command(TARGET test_xmeta POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/test/xmeta/example/midl3test"
        $<TARGET_FILE_DIR:test_xmeta>/midl3test)
endif()

target_sources(test_xmeta PUBLIC
    pch.cpp
    test_lexer.cpp
    test_symbol_table.cpp
    test_midl_compat.cpp
    xlang_test_listener.cpp
)

if (WIN32)
    target_sources(test_xmeta PUBLIC
        test_xmeta_emit.cpp
    )
endif()

target_include_directories(test_xmeta
    PUBLIC ${XLANG_TEST_INC_PATH} ${CMAKE_CURRENT_SOURCE_DIR})


target_link_libraries(test_xmeta xmeta_model)
if (WIN32)
    target_link_libraries(test_xmeta xmeta_antlr xmeta_windows)
endif()


if (MSVC)
    TARGET_CONFIG_MSVC_PCH(test_xmeta pch.cpp pch.h)
    if (WIN32)
        target_link_libraries(test_xmeta windowsapp ole32 WindowsApp.lib comsuppw.lib)
    endif()
endif()

if (WIN32)
    get_target_property(xmeta_path foundation_metadata Foundation_xmeta)
    add_custom_command(TARGET test_xmeta POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${xmeta_path}
        ${CMAKE_CURRENT_BINARY_DIR}/Foundation.xmeta
    )
endif()

target_sources(test_xmeta PUBLIC main.cpp)
install(TARGETS test_xmeta DESTINATION "test/xmeta")
install(FILES $<TARGET_PDB_FILE:test_xmeta> DESTINATION "test/xmeta" OPTIONAL)
