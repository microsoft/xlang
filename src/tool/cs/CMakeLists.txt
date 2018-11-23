cmake_minimum_required(VERSION 3.9)

project(cswinrt)

add_executable(cswinrt "")
target_sources(cswinrt PUBLIC main.cpp pch.cpp)
target_include_directories(cswinrt PUBLIC ${XLANG_LIBRARY_PATH} ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR})

if (WIN32)
    TARGET_CONFIG_MSVC_PCH(cswinrt pch.cpp pch.h)
    target_link_libraries(cswinrt windowsapp ole32)
else()
    target_link_libraries(cswinrt c++ c++abi c++experimental)
    target_link_libraries(cswinrt -lpthread)
endif()
