project(cppwinrt)

add_executable(cppwinrt "")
target_sources(cppwinrt PUBLIC main.cpp pch.cpp "${PROJECT_BINARY_DIR}/strings.cpp")
target_include_directories(cppwinrt PUBLIC ${XLANG_LIBRARY_PATH} ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR})

GENERATE_STRING_LITERAL_FILES("${PROJECT_SOURCE_DIR}/strings/*.h" "strings" "xlang::strings" cppwinrt)

if (WIN32)
    TARGET_CONFIG_MSVC_PCH(cppwinrt pch.cpp pch.h)
    target_link_libraries(cppwinrt windowsapp ole32 shlwapi)
else()
    target_link_libraries(cppwinrt c++ c++abi c++experimental)
    target_link_libraries(cppwinrt -lpthread)
endif()
