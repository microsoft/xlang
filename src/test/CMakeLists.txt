set(XLANG_TEST_INC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/inc")
find_path(CATCH2_INCLUDE_DIR catch.hpp)

if (WIN32)
    add_subdirectory(component)
    add_subdirectory(cpp)
    add_subdirectory(platform)
    add_subdirectory(python)
endif()
