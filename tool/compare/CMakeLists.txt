cmake_minimum_required(VERSION 3.9)

project(compare)

add_executable(compare)
target_sources(compare PUBLIC main.cpp)
target_include_directories(compare PUBLIC ${LW_LIBRARY_PATH})

if (WIN32)
    target_link_libraries(compare windowsapp ole32)
else ()
    target_link_libraries(compare c++ c++abi c++experimental)
endif()
