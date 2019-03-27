cmake_minimum_required(VERSION 3.9)

project(cppxlang)

add_executable(cppxlang "")
target_sources(cppxlang PUBLIC main.cpp pch.cpp "${PROJECT_BINARY_DIR}/strings.cpp")
target_include_directories(cppxlang PUBLIC ${XLANG_LIBRARY_PATH} ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR})

GENERATE_STRING_LITERAL_FILES("${PROJECT_SOURCE_DIR}/strings/*" "strings" "xlang::strings" cppxlang)

if (WIN32)
    TARGET_CONFIG_MSVC_PCH(cppxlang pch.cpp pch.h)
    target_compile_options(cppxlang PUBLIC /await)
    target_link_libraries(cppxlang windowsapp ole32 shlwapi)
else()
    target_link_libraries(cppxlang c++ c++abi c++experimental)
    target_link_libraries(cppxlang -lpthread)
endif()

string(APPEND CMAKE_CXX_FLAGS " -D \"XLANG_VERSION_STRING=\\\"${XLANG_BUILD_VERSION}\\\"\"")

add_subdirectory(test_base)
