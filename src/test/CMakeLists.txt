cmake_minimum_required(VERSION 3.9)

set(XLANG_TEST_INC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/inc")

add_subdirectory(platform)

if (WIN32)
    add_subdirectory(component)
    add_subdirectory(cpp)
    add_subdirectory(python)
endif()

