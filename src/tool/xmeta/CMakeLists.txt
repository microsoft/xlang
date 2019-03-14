cmake_minimum_required(VERSION 3.9)

project(xmeta)

set(xmeta_gen_generated
  ${PROJECT_SOURCE_DIR}/cpp/generated/xlang_lexer.cpp
  ${PROJECT_SOURCE_DIR}/cpp/generated/xlang_parser.cpp
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
  )

set(xmeta_gen_src
  ${PROJECT_SOURCE_DIR}/main.cpp
  ${xmeta_gen_generated}       
  )

add_executable(xmeta ${xmeta_gen_src})

target_link_libraries(xmeta "${PROJECT_SOURCE_DIR}/libs/antlr4-runtime.lib")

add_custom_command(TARGET xmeta PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
  "${PROJECT_SOURCE_DIR}/libs/antlr4-runtime.dll"
  $<TARGET_FILE_DIR:xmeta>)

