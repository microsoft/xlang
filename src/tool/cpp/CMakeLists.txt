cmake_minimum_required(VERSION 3.9)

add_subdirectory(cppx)

if (WIN32)
add_subdirectory(basex)
add_subdirectory(cppwinrt)
add_subdirectory(basewinrt)
endif()
