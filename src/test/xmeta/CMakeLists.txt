cmake_minimum_required(VERSION 3.9)

project(test_xmeta)

include_directories(
    ${CMAKE_SOURCE_DIR}/tool/xmeta/antlr4-runtime/
    ${CMAKE_SOURCE_DIR}/tool/xmeta/antlr4-runtime/misc
    ${CMAKE_SOURCE_DIR}/tool/xmeta/antlr4-runtime/atn
    ${CMAKE_SOURCE_DIR}/tool/xmeta/antlr4-runtime/dfa
    ${CMAKE_SOURCE_DIR}/tool/xmeta/antlr4-runtime/tree
    ${CMAKE_SOURCE_DIR}/tool/xmeta/antlr4-runtime/support
    ${CMAKE_SOURCE_DIR}/tool/xmeta/
    ${CMAKE_SOURCE_DIR}/tool/xmeta/libs/
    ${CMAKE_SOURCE_DIR}/tool/xmeta/cpp/generated
    ${CMAKE_SOURCE_DIR}/tool/xmeta/cpp/models/
    ${CMAKE_SOURCE_DIR}/tool/xmeta/cpp/
)

add_executable(test_xmeta "")
target_link_libraries(test_xmeta "WindowsApp.lib")
target_link_libraries(test_xmeta "comsuppw.lib")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_link_libraries(test_xmeta "${CMAKE_SOURCE_DIR}/tool/xmeta/antlr4-runtime/antlrlib/x86debug/antlr4-runtime.lib")
    add_custom_command(TARGET test_xmeta PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/tool/xmeta/antlr4-runtime/antlrlib/x86debug/antlr4-runtime.dll"
        $<TARGET_FILE_DIR:test_xmeta>)
else()
    target_link_libraries(test_xmeta "${CMAKE_SOURCE_DIR}/tool/xmeta/antlr4-runtime/antlrlib/x86debug/antlr4-runtime.lib")
    add_custom_command(TARGET test_xmeta PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/tool/xmeta/antlr4-runtime/antlrlib/x86debug/antlr4-runtime.dll"
        $<TARGET_FILE_DIR:test_xmeta>)
endif()

if (EXISTS "${CMAKE_SOURCE_DIR}/test/xmeta/example/midl3test")
add_custom_command(TARGET test_xmeta POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/test/xmeta/example/midl3test"
    $<TARGET_FILE_DIR:test_xmeta>/midl3test)
endif()

target_sources(test_xmeta
    PUBLIC pch.cpp testlexer.cpp testmidlcompat.cpp testxmetaemit.cpp)

target_include_directories(test_xmeta
    PUBLIC ${XLANG_LIBRARY_PATH} ${XLANG_TEST_INC_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../output/component/source)

if (MSVC)
    TARGET_CONFIG_MSVC_PCH(test_xmeta pch.cpp pch.h)
    target_link_libraries(test_xmeta windowsapp ole32)
endif()

set(xmeta_gen_generated
    ${CMAKE_SOURCE_DIR}/tool/xmeta/cpp/generated/XlangLexer.cpp
    ${CMAKE_SOURCE_DIR}/tool/xmeta/cpp/generated/XlangParser.cpp
    ${CMAKE_SOURCE_DIR}/test/xmeta/xlang_test_listener.cpp
    ${CMAKE_SOURCE_DIR}/tool/xmeta/xmeta_emit.cpp
    ${CMAKE_SOURCE_DIR}/tool/xmeta/xlang_model_walker.cpp
 )

target_sources(test_xmeta PUBLIC main.cpp ${xmeta_gen_generated})

# https://stackoverflow.com/questions/35310117/debug-assertion-failed-expression-acrt-first-block-header
# Temporary measure
target_compile_options(test_xmeta PRIVATE "/MD$<$<CONFIG:Debug>:d>")

install(TARGETS test_xmeta DESTINATION "test/xmeta")
if (WIN32)
    install(FILES $<TARGET_PDB_FILE:test_xmeta> DESTINATION "test/xmeta" OPTIONAL)
endif ()

