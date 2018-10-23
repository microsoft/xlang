cmake_minimum_required(VERSION 3.9)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/inc")

if (WIN32)
    add_subdirectory(il)
    add_subdirectory(cpp)
    # TODO: enable platform tests compiling on Ubuntu
    add_subdirectory(platform)
    add_subdirectory(python)
endif()