cmake_minimum_required(VERSION 3.9)

project(foowinrt)

add_executable(foowinrt "")
target_sources(foowinrt PUBLIC main.cpp pch.cpp)
target_include_directories(foowinrt PUBLIC ${XLANG_LIBRARY_PATH} ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR})

if (WIN32)
    TARGET_CONFIG_MSVC_PCH(foowinrt pch.cpp pch.h)
    target_compile_options(foowinrt PUBLIC /await)
    target_link_libraries(foowinrt windowsapp ole32)
else()
    target_link_libraries(foowinrt c++ c++abi c++experimental)
    target_link_libraries(foowinrt -lpthread)
endif()
