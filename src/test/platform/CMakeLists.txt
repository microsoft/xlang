cmake_minimum_required(VERSION 3.9)

project(test_platform)

add_executable(test_platform "")
target_sources(test_platform
    PUBLIC pch.cpp memory.cpp string.cpp activation.cpp)

target_include_directories(test_platform
    PUBLIC ${XLANG_LIBRARY_PATH} ${XLANG_TEST_INC_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../output/component/source)

target_link_libraries(test_platform pal)
RPATH_ORIGIN(test_platform)

if (MSVC)
    TARGET_CONFIG_MSVC_PCH(test_platform pch.cpp pch.h)
    target_link_libraries(test_platform windowsapp ole32)
endif()

target_sources(test_platform PUBLIC main.cpp)
add_dependencies(test_platform abi_test_component)

install(TARGETS test_platform DESTINATION "test/platform")
if (WIN32)
    install(FILES $<TARGET_PDB_FILE:test_platform> DESTINATION "test/platform" OPTIONAL)
endif ()
