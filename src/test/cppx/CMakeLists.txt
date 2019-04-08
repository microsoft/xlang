cmake_minimum_required(VERSION 3.9)

project(test_cppx)

add_executable(test_cppx "")
target_sources(test_cppx
    PUBLIC pch.cpp hstring.cpp)

target_include_directories(test_cppx
    PRIVATE ${XLANG_LIBRARY_PATH} ${XLANG_TEST_INC_PATH} ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(test_cppx pal)
RPATH_ORIGIN(test_cppx)

if (MSVC)
    TARGET_CONFIG_MSVC_PCH(test_cppx pch.cpp pch.h)
    target_link_libraries(test_cppx windowsapp ole32)
endif()

target_sources(test_cppx PUBLIC main.cpp)

install(TARGETS test_cppx DESTINATION "test/platform")
if (WIN32)
    install(FILES $<TARGET_PDB_FILE:test_cppx> DESTINATION "test/platform" OPTIONAL)
endif ()

add_custom_target(test_cppx_base_projection
    COMMAND cppxlang -base -out "${CMAKE_CURRENT_BINARY_DIR}")

add_dependencies(test_cppx test_cppx_base_projection)
