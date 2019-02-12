project(abi)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_compile_options(-Wno-missing-field-initializers)
endif()

add_executable(abi)
target_sources(abi PUBLIC
    abi_writer.cpp
    main.cpp
    metadata_cache.cpp
    pch.cpp
    types.cpp
    "${PROJECT_BINARY_DIR}/strings.cpp")
target_include_directories(abi PUBLIC ${XLANG_LIBRARY_PATH} ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR})

GENERATE_STRING_LITERAL_FILES("${PROJECT_SOURCE_DIR}/strings/*.h" "strings" "xlang::strings" abi)

if (WIN32)
    TARGET_CONFIG_MSVC_PCH(abi pch.cpp pch.h)
    target_link_libraries(abi windowsapp ole32 shlwapi)
else()
    target_link_libraries(abi c++ c++abi c++experimental)
    target_link_libraries(abi -lpthread)
endif()
