cmake_minimum_required(VERSION 3.9)

project(cpp)

add_executable(cpp "")
target_sources(cpp PUBLIC main.cpp pch.cpp)
target_include_directories(cpp PUBLIC ${XLANG_LIBRARY_PATH})

if (WIN32)
    TARGET_CONFIG_MSVC_PCH(cpp pch.cpp pch.h)
    target_compile_options(cpp PUBLIC /await)
    target_link_libraries(cpp windowsapp ole32)
else()
    target_link_libraries(cpp c++ c++abi c++experimental)
    target_link_libraries(cpp -lpthread)
endif()
