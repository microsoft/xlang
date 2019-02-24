cmake_minimum_required(VERSION 3.9)

add_subdirectory(abi)
add_subdirectory(idl)
add_subdirectory(python)

if (WIN32)
add_subdirectory(cppwinrt)
endif()
