cmake_minimum_required(VERSION 3.9)

if (WIN32)

project(javart)

add_executable(javart "")
target_sources(javart PUBLIC main.cpp pch.cpp "${PROJECT_BINARY_DIR}/strings.cpp")
target_include_directories(javart PUBLIC ${XLANG_LIBRARY_PATH} ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR})

GENERATE_STRING_LITERAL_FILES("${PROJECT_SOURCE_DIR}/strings/*.h" "strings" "xlang::strings" javart)

add_custom_target(gen_javart_strings ALL DEPENDS "${PROJECT_BINARY_DIR}/strings.cpp")

TARGET_CONFIG_MSVC_PCH(javart pch.cpp pch.h)
target_link_libraries(javart windowsapp ole32 shlwapi)

endif()
