cmake_minimum_required(VERSION 3.9)

project(test_cpp)

add_executable(test_cpp "")
target_sources(test_cpp PUBLIC pch.cpp meta_reader_filter.cpp meta_reader_cache.cpp cmd_reader.cpp simple.cpp)
target_include_directories(test_cpp PUBLIC ${XLANG_LIBRARY_PATH} ..\\output\\foundation ..\\output\\simple)

if (MSVC)
    TARGET_CONFIG_MSVC_PCH(test_cpp pch.cpp pch.h)
    target_link_libraries(test_cpp windowsapp ole32)
else()
    target_link_libraries(test_cpp c++ c++abi c++experimental)
    target_link_libraries(test_cpp -lpthread)
endif()

target_sources(test_cpp PUBLIC main.cpp)
add_dependencies(test_cpp test_il)
