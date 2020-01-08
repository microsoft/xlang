set(WINRT_INCLUDE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/include")

if (WIN32)
add_subdirectory(activation)
add_subdirectory(detours)
add_subdirectory(example)
add_subdirectory(test)
endif()
