cmake_minimum_required(VERSION 3.9)

project(xmeta)

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

add_executable(xmeta main.cpp)

target_link_libraries(xmeta "${PROJECT_SOURCE_DIR}/libs/antlr4-runtime.lib")

add_custom_command(TARGET xmeta PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
  "${PROJECT_SOURCE_DIR}/libs/antlr4-runtime.dll"
  $<TARGET_FILE_DIR:xmeta>)

install(TARGETS xmeta DESTINATION "test/xmeta")
if (WIN32)
    install(FILES $<TARGET_PDB_FILE:xmeta> DESTINATION "test/xmeta" OPTIONAL)
endif ()


