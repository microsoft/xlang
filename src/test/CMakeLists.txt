cmake_minimum_required(VERSION 3.9)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/inc")

add_subdirectory(platform)

if (WIN32)
    add_subdirectory(component)
    add_subdirectory(cpp)
    add_subdirectory(python)
endif()
