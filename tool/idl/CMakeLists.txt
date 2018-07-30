cmake_minimum_required(VERSION 3.9)

project(idl)

add_executable(idl "")
target_sources(idl PUBLIC main.cpp pch.cpp)
target_include_directories(idl PUBLIC ${XLANG_LIBRARY_PATH})

if (WIN32)
endif()

if (WIN32)
    TARGET_CONFIG_MSVC_PCH(idl pch.cpp pch.h)
    target_compile_options(idl PUBLIC /await)
    target_link_libraries(idl windowsapp ole32)
else()
    target_link_libraries(idl c++ c++abi c++experimental)
    target_link_libraries(idl -lpthread)
endif()
