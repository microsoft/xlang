cmake_minimum_required(VERSION 3.9)

project(pal)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

set(sources string_abi.cpp string_base.cpp activation_abi.cpp)

if (WIN32)
    set(sources ${sources} win32_memory.cpp win32_string_convert.cpp win32_activation.cpp)
else()
    set(sources ${sources} common_memory.cpp)
endif()

add_definitions(-DXLANG_PAL_EXPORTS)
add_library(pal SHARED ${sources})
target_include_directories(pal PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(pal PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/published)
