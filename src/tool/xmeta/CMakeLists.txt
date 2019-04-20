cmake_minimum_required(VERSION 3.9)

project(xmeta)

set(xmeta_gen_generated
    ${PROJECT_SOURCE_DIR}/cpp/generated/XlangLexer.cpp
    ${PROJECT_SOURCE_DIR}/cpp/generated/XlangParser.cpp
 )

include_directories(
    ${PROJECT_SOURCE_DIR}/antlr4-runtime/
    ${PROJECT_SOURCE_DIR}/antlr4-runtime/misc
    ${PROJECT_SOURCE_DIR}/antlr4-runtime/atn
    ${PROJECT_SOURCE_DIR}/antlr4-runtime/dfa
    ${PROJECT_SOURCE_DIR}/antlr4-runtime/tree
    ${PROJECT_SOURCE_DIR}/antlr4-runtime/support
    ${PROJECT_SOURCE_DIR}/
    ${PROJECT_SOURCE_DIR}/libs/
    ${PROJECT_SOURCE_DIR}/cpp/generated/
    ${PROJECT_SOURCE_DIR}/cpp/models/
    ${PROJECT_SOURCE_DIR}/cpp/
  )

set(xmeta_gen_src
    ${PROJECT_SOURCE_DIR}/main.cpp
    ${PROJECT_SOURCE_DIR}/xmeta_emit.cpp
    ${PROJECT_SOURCE_DIR}/xlang_model_walker.cpp
    ${xmeta_gen_generated}       
 )

add_executable(xmeta ${xmeta_gen_src})
target_link_libraries(xmeta "WindowsApp.lib" "comsuppw.lib")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_link_libraries(xmeta "${PROJECT_SOURCE_DIR}/antlr4-runtime/antlrlib/x86debug/antlr4-runtime.lib")
    add_custom_command(TARGET xmeta PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PROJECT_SOURCE_DIR}/antlr4-runtime/antlrlib/x86debug/antlr4-runtime.dll"
        $<TARGET_FILE_DIR:xmeta>)
    # https://stackoverflow.com/questions/35310117/debug-assertion-failed-expression-acrt-first-block-header
    # Temporary measure
target_compile_options(xmeta PRIVATE "/MD$<$<CONFIG:Debug>:d>")
else()
    target_link_libraries(xmeta "${PROJECT_SOURCE_DIR}/antlr4-runtime/antlrlib/x86release/antlr4-runtime.lib")
    add_custom_command(TARGET xmeta PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PROJECT_SOURCE_DIR}/antlr4-runtime/antlrlib/x86release/antlr4-runtime.dll"
        $<TARGET_FILE_DIR:xmeta>)
endif()

