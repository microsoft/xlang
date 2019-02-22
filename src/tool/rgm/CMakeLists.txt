cmake_minimum_required(VERSION 3.9)

project(rgm)

add_executable(rgm "")
target_sources(rgm PUBLIC main.cpp pch.cpp)
target_include_directories(rgm PUBLIC ${XLANG_LIBRARY_PATH} ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR})

if (WIN32)
    TARGET_CONFIG_MSVC_PCH(rgm pch.cpp pch.h)
    target_compile_options(rgm PUBLIC /await)
    target_link_libraries(rgm windowsapp ole32 shlwapi)
else()
    target_link_libraries(rgm c++ c++abi c++experimental)
    target_link_libraries(rgm -lpthread)
endif()
