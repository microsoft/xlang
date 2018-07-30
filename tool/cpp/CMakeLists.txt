cmake_minimum_required(VERSION 3.9)

project(cpp)

add_executable(cpp ${sources})
configure_file(cpp.vcxproj.user ${CMAKE_CURRENT_BINARY_DIR}/cpp.vcxproj.user)
target_sources(cpp PUBLIC main.cpp pch.cpp)
target_include_directories(cpp PUBLIC ${LW_LIBRARY_PATH})
target_compile_options(cpp PUBLIC /await)

if (WIN32)
    target_link_libraries(cpp windowsapp ole32)
else ()
    target_link_libraries(cpp c++ c++abi c++experimental)
    target_link_libraries(cpp WinRT.PAL.Static)
endif()
