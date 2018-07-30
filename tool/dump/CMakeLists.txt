cmake_minimum_required(VERSION 3.9)

project(dump)

add_executable(dump)
target_sources(dump PUBLIC main.cpp pch.cpp)
TARGET_CONFIG_MSVC_PCH(dump pch.cpp pch.h)
target_include_directories(dump PUBLIC ${LW_LIBRARY_PATH})
target_link_libraries(dump windowsapp ole32)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Windows.Foundation.winmd
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})