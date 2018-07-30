cmake_minimum_required(VERSION 3.9)

project(cpp)

add_executable(cpp)
target_sources(cpp PUBLIC main.cpp pch.cpp)
TARGET_CONFIG_MSVC_PCH(cpp pch.cpp pch.h)
target_include_directories(cpp PUBLIC ${LW_LIBRARY_PATH})
target_compile_options(cpp PUBLIC /await)
target_link_libraries(cpp windowsapp ole32)
