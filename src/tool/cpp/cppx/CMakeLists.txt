cmake_minimum_required(VERSION 3.9)

project(cppx)

add_executable(cppx "")
target_sources(cppx PUBLIC main.cpp pch.cpp "${PROJECT_BINARY_DIR}/strings.cpp")
target_include_directories(cppx PUBLIC ${XLANG_LIBRARY_PATH} ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR})

GENERATE_STRING_LITERAL_FILES("${PROJECT_SOURCE_DIR}/strings/*.h" "strings" "xlang::strings" cppx)

if (WIN32)
    TARGET_CONFIG_MSVC_PCH(cppx pch.cpp pch.h)
    target_link_libraries(cppx windowsapp ole32)
else()
    target_link_libraries(cppx c++ c++abi c++experimental)
    target_link_libraries(cppx -lpthread)
endif()
