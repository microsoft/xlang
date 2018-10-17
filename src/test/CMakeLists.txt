cmake_minimum_required(VERSION 3.9)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/inc")

add_subdirectory(il)
add_subdirectory(cpp)
add_subdirectory(platform)
add_subdirectory(python)
add_subdirectory(components)
