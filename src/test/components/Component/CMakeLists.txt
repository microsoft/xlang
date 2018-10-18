cmake_minimum_required(VERSION 3.9)

project(Component)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

add_library(Component SHARED module.cpp Class.cpp module.def)
target_include_directories(Component
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../output/component/source)

CONSUME_PAL(Component)

if (MSVC)
    target_link_libraries(Component windowsapp ole32)
else()
    target_link_libraries(Component c++ c++abi c++experimental)
    target_link_libraries(Component -lpthread)
endif()

install(TARGETS Component DESTINATION "test_platform")
