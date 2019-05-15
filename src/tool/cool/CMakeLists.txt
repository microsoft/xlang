project(coolrt)

add_executable(coolrt "")
target_sources(coolrt PUBLIC main.cpp pch.cpp "${PROJECT_BINARY_DIR}/strings.cpp")
target_include_directories(coolrt PUBLIC ${XLANG_LIBRARY_PATH} ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR})

GENERATE_STRING_LITERAL_FILES("${PROJECT_SOURCE_DIR}/strings/*" "strings" "coolrt::strings" coolrt)

if (WIN32)
    TARGET_CONFIG_MSVC_PCH(coolrt pch.cpp pch.h)
    target_link_libraries(coolrt windowsapp ole32 shlwapi)
else()
    target_link_libraries(coolrt c++ c++abi c++experimental)
    target_link_libraries(coolrt -lpthread)
endif()
