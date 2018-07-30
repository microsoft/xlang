cmake_minimum_required(VERSION 3.9)

project(idl)

add_executable(idl)
target_sources(idl PUBLIC main.cpp pch.cpp)
TARGET_CONFIG_MSVC_PCH(idl pch.cpp pch.h)
target_include_directories(idl PUBLIC ${LW_LIBRARY_PATH})
target_compile_options(idl PUBLIC /await)
target_link_libraries(idl windowsapp ole32)

