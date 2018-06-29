cmake_minimum_required(VERSION 3.5)

project(dump)

set(sources main.cpp)

include_directories("../../library")

add_executable(dump ${sources})

if (WIN32)
    target_link_libraries(dump windowsapp ole32)
else ()
    target_link_libraries(dump c++ c++abi c++experimental)
    target_link_libraries(dump WinRT.PAL.Static)
endif()

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Windows.Foundation.winmd
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})