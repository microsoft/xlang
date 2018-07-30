cmake_minimum_required(VERSION 3.9)

project(compare)

add_executable(compare)
target_sources(compare PUBLIC main.cpp pch.cpp)
TARGET_CONFIG_MSVC_PCH(compare pch.cpp pch.h)
target_include_directories(compare PUBLIC ${XLANG_LIBRARY_PATH})
target_link_libraries(compare windowsapp ole32)
