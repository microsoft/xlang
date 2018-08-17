cmake_minimum_required(VERSION 3.9)

project(cpp)

add_executable(cpp "")
target_sources(cpp PUBLIC main.cpp pch.cpp "${PROJECT_BINARY_DIR}/strings.cpp")
target_include_directories(cpp PUBLIC ${XLANG_LIBRARY_PATH} ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR})

GENERATE_STRING_LITERAL_FILES("${PROJECT_SOURCE_DIR}/strings/*.h" "strings" "xlang::strings" cpp)

if (WIN32)
    TARGET_CONFIG_MSVC_PCH(cpp pch.cpp pch.h)
    target_link_libraries(cpp windowsapp ole32)
else()
    target_link_libraries(cpp c++ c++abi c++experimental)
    target_link_libraries(cpp -lpthread)
endif()

# The new cpp.exe tool runs xplat, but the code it generates (currently) is not
# TODO: enable base subdirectory for all platforms once base strings have been updated for xplat
if (WIN32)
    add_subdirectory(base)
endif()
