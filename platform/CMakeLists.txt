cmake_minimum_required(VERSION 3.9)

project(pal)

if (WIN32)
    set(sources win32_strings.cpp win32_exports.def)
else()
    set(sources common_memory.cpp)
endif()

add_library(pal SHARED ${sources})
target_include_directories(pal PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
add_definitions(-DXLANG_PAL_EXPORTS)
