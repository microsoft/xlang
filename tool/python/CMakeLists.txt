cmake_minimum_required(VERSION 3.9)

project(pywinrt)

add_executable(pywinrt "")
target_sources(pywinrt PUBLIC main.cpp pch.cpp "${PROJECT_BINARY_DIR}/strings.cpp")
target_include_directories(pywinrt PUBLIC ${XLANG_LIBRARY_PATH} ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR})

GENERATE_STRING_LITERAL_FILES("${PROJECT_SOURCE_DIR}/strings/*.py" "strings" "xlang::strings" pywinrt)

if (WIN32)
    TARGET_CONFIG_MSVC_PCH(pywinrt pch.cpp pch.h)
    target_compile_options(pywinrt PUBLIC /await)
    target_link_libraries(pywinrt windowsapp ole32)
else()
    target_link_libraries(pywinrt c++ c++abi c++experimental)
    target_link_libraries(pywinrt -lpthread)
endif()
