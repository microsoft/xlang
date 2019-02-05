cmake_minimum_required(VERSION 3.9)

project(test_platform)

add_executable(test_platform "")
target_sources(test_platform
    PUBLIC pch.cpp memory.cpp string.cpp activation.cpp)

target_include_directories(test_platform
    PUBLIC ${XLANG_LIBRARY_PATH} ${XLANG_TEST_INC_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../output/component/source)

CONSUME_PAL(test_platform)

if (MSVC)
    TARGET_CONFIG_MSVC_PCH(test_platform pch.cpp pch.h)
    target_link_libraries(test_platform windowsapp ole32)
else()
    target_link_libraries(test_platform c++ c++abi c++experimental)
    target_link_libraries(test_platform -lpthread)
endif()

target_sources(test_platform PUBLIC main.cpp)

# install(TARGETS test_platform DESTINATION "test_platform")
