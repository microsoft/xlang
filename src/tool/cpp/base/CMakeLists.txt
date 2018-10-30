cmake_minimum_required(VERSION 3.9)

project(cpp_base)

add_executable(cpp_base "")
target_sources(cpp_base PUBLIC main.cpp)
target_include_directories(cpp_base PUBLIC ${PROJECT_SOURCE_DIR}/../strings)

if (WIN32)
    target_link_libraries(cpp_base windowsapp ole32)
else()
    target_link_libraries(cpp_base c++ c++abi c++experimental)
    target_link_libraries(cpp_base -lpthread)
endif()
