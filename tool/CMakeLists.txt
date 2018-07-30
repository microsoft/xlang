cmake_minimum_required(VERSION 3.9)

add_subdirectory(cpp)
add_subdirectory(dump)
add_subdirectory(idl)

if (WIN32)
    add_subdirectory(compare)
endif()
