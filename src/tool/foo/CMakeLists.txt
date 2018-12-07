cmake_minimum_required(VERSION 3.9)

find_path(FFI_INCLUDE_DIR ffi.h)
find_library(FFI_LIBRARY_PATH libffi)

project(foo)

add_executable(foo "")
target_sources(foo PUBLIC main.cpp pch.cpp)
target_include_directories(foo PUBLIC ${XLANG_LIBRARY_PATH} ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR} ${FFI_INCLUDE_DIR})
target_link_libraries(foo ${FFI_LIBRARY_PATH})

if (WIN32)
    TARGET_CONFIG_MSVC_PCH(foo pch.cpp pch.h)
    target_compile_options(foo PUBLIC /await)
    target_link_libraries(foo windowsapp ole32)
else()
    target_link_libraries(foo c++ c++abi c++experimental)
    target_link_libraries(foo -lpthread)
endif()
