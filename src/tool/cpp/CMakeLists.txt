cmake_minimum_required(VERSION 3.9)

add_subdirectory(cppx)
add_subdirectory(basex)

if (WIN32)
add_subdirectory(cppwinrt)
add_subdirectory(basewinrt)
endif()
