set(WINRT_INCLUDE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/include")

if(NOT DEFINED REGFREE_WINRT_BUILD_VERSION)
set(REGFREE_WINRT_BUILD_VERSION "1.0.0")
endif()

if (WIN32)
add_subdirectory(activation)
add_subdirectory(detours)
add_subdirectory(example)
add_subdirectory(test)
add_subdirectory(nuget)
endif()
