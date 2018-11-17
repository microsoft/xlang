cmake_minimum_required(VERSION 3.9)

project(basewinrt)

add_executable(basewinrt "")
target_sources(basewinrt PUBLIC main.cpp)
target_include_directories(basewinrt PUBLIC ${PROJECT_SOURCE_DIR}/../cppwinrt/strings)

if (WIN32)
    target_link_libraries(basewinrt windowsapp ole32)
else()
    target_link_libraries(basewinrt c++ c++abi c++experimental)
    target_link_libraries(basewinrt -lpthread)
endif()
