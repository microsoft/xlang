cmake_minimum_required(VERSION 3.9)

project(dump)

add_executable(dump ${sources})
target_sources(dump PUBLIC main.cpp)
target_include_directories(dump PUBLIC ${LW_LIBRARY_PATH})

if (WIN32)
    target_link_libraries(dump windowsapp ole32)
else ()
    target_link_libraries(dump c++ c++abi c++experimental)
endif()

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Windows.Foundation.winmd
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})