cmake_minimum_required(VERSION 3.9)

project(meta_reader)

add_executable(meta_reader "")
target_sources(meta_reader PUBLIC main.cpp pch.cpp)
target_include_directories(meta_reader PUBLIC ${XLANG_LIBRARY_PATH})

set_target_properties(meta_reader PROPERTIES EXCLUDE_FROM_ALL TRUE)

if (WIN32)
    TARGET_CONFIG_MSVC_PCH(meta_reader pch.cpp pch.h)
    target_link_libraries(meta_reader windowsapp ole32 shlwapi)
else()
    target_link_libraries(meta_reader c++ c++abi c++experimental)
    target_link_libraries(meta_reader -lpthread)
endif()
