cmake_minimum_required(VERSION 3.5)

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_definitions(-DDEBUG)
endif()

if (WIN32)
    add_compile_options(/std:c++17)
    add_compile_options(/EHsc)
    add_compile_options(/permissive-)
else()
    add_compile_options(-std=c++17)
    add_compile_options(-stdlib=libc++)
    add_compile_options(-pthread)
    add_compile_options(-fexceptions)
    add_compile_options(-fPIC)
    add_compile_options(-g -gdwarf-4)  

    SET( CMAKE_SHARED_LINKER_FLAGS  "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined" ) 
endif()

if (NOT WIN32)
add_subdirectory(platform)
endif()

add_subdirectory(tool)
