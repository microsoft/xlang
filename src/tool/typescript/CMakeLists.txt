cmake_minimum_required(VERSION 3.9)

project(tsrt)

add_executable(tsrt "")
target_sources(tsrt PUBLIC main.cpp pch.cpp "${PROJECT_BINARY_DIR}/strings.cpp")
target_include_directories(tsrt PUBLIC ${XLANG_LIBRARY_PATH} ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR})

GENERATE_STRING_LITERAL_FILES("${PROJECT_SOURCE_DIR}/strings/*" "strings" "xlang::strings" tsrt)

if (WIN32)
    TARGET_CONFIG_MSVC_PCH(tsrt pch.cpp pch.h)
    target_compile_options(tsrt PUBLIC /await)
    target_link_libraries(tsrt windowsapp ole32 shlwapi)
else()
    target_link_libraries(tsrt c++ c++abi c++experimental)
    target_link_libraries(tsrt -lpthread)
endif()

string(APPEND CMAKE_CXX_FLAGS " -D \"XLANG_VERSION_STRING=\\\"${XLANG_BUILD_VERSION}\\\"\"")