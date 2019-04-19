cmake_minimum_required(VERSION 3.9)

project(xmeta)

set(xmeta_gen_generated
    ${PROJECT_SOURCE_DIR}/cpp/generated/XlangLexer.cpp
    ${PROJECT_SOURCE_DIR}/cpp/generated/XlangParser.cpp
 )

set(xmeta_cpp
    ${PROJECT_SOURCE_DIR}/cpp/ast_to_st_listener.cpp
    ${PROJECT_SOURCE_DIR}/cpp/symbol_table.cpp
)

include_directories(
    ${PROJECT_SOURCE_DIR}/antlr4-runtime/
    ${PROJECT_SOURCE_DIR}/antlr4-runtime/misc/
    ${PROJECT_SOURCE_DIR}/antlr4-runtime/atn/
    ${PROJECT_SOURCE_DIR}/antlr4-runtime/dfa/
    ${PROJECT_SOURCE_DIR}/antlr4-runtime/tree/
    ${PROJECT_SOURCE_DIR}/antlr4-runtime/support/
    ${PROJECT_SOURCE_DIR}/
    ${PROJECT_SOURCE_DIR}/libs/
    ${PROJECT_SOURCE_DIR}/cpp/
    ${PROJECT_SOURCE_DIR}/cpp/generated/
    ${PROJECT_SOURCE_DIR}/cpp/models/
  )

set(xmeta_src
    ${PROJECT_SOURCE_DIR}/main.cpp
    ${xmeta_gen_generated}
    ${xmeta_cpp}
  )

add_executable(xmeta "")
target_sources(xmeta PUBLIC ${xmeta_src})

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_link_libraries(xmeta "${PROJECT_SOURCE_DIR}/antlr4-runtime/antlrlib/x86debug/antlr4-runtime.lib")
    add_custom_command(TARGET xmeta PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PROJECT_SOURCE_DIR}/antlr4-runtime/antlrlib/x86debug/antlr4-runtime.dll"
        $<TARGET_FILE_DIR:xmeta>)
else()
    target_link_libraries(xmeta "${PROJECT_SOURCE_DIR}/antlr4-runtime/antlrlib/x86release/antlr4-runtime.lib")
    add_custom_command(TARGET xmeta PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PROJECT_SOURCE_DIR}/antlr4-runtime/antlrlib/x86release/antlr4-runtime.dll"
        $<TARGET_FILE_DIR:xmeta>)
endif()
