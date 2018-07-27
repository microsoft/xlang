cmake_minimum_required(VERSION 3.9)

project(idl)

add_executable(idl ${sources})
target_sources(idl PUBLIC main.cpp pch.cpp)
target_include_directories(idl PUBLIC ${LW_LIBRARY_PATH})
target_compile_options(idl PUBLIC /await)

if (WIN32)
    target_link_libraries(idl windowsapp ole32)
else ()
    target_link_libraries(idl c++ c++abi c++experimental)
    target_link_libraries(idl WinRT.PAL.Static)
endif()
