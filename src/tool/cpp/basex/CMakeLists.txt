cmake_minimum_required(VERSION 3.9)

project(basex)

add_executable(basex "")
target_sources(basex PUBLIC main.cpp)
target_include_directories(basex PUBLIC ${PROJECT_SOURCE_DIR}/../cppx/strings)

if (WIN32)
    target_link_libraries(basex windowsapp ole32)
else()
    target_link_libraries(basex c++ c++abi c++experimental)
    target_link_libraries(basex -lpthread)
endif()
