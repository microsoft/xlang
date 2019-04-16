project(pal)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

set(sources string_abi.cpp string_base.cpp activation_abi.cpp)

if (WIN32)
    set(sources ${sources} win32_memory.cpp win32_string_convert.cpp win32_activation.cpp)
else()
    set(sources ${sources} common_memory.cpp common_string_convert.cpp common_activation.cpp)
endif()

add_definitions(-DXLANG_PAL_EXPORTS)
add_library(pal SHARED ${sources})
target_include_directories(pal PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/helpers)
target_include_directories(pal PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/published)
set_target_properties(pal PROPERTIES OUTPUT_NAME "xlangpal")
RPATH_ORIGIN(pal)

if (NOT WIN32)
    target_link_libraries(pal -ldl)
    target_link_libraries(pal c++ c++abi)
    target_link_libraries(pal -lpthread)
endif()

install(TARGETS pal DESTINATION "test/platform")
if (WIN32)
    add_custom_target(xlang_foundation
        COMMAND ilasm /DLL /mdv="WindowsRuntime 1.4" /output="${CMAKE_CURRENT_BINARY_DIR}/xlang_foundation.xmeta" "${CMAKE_CURRENT_SOURCE_DIR}/xlang_foundation.il")
    install(TARGETS xlang_foundation "test/platform")
    install(FILES $<TARGET_PDB_FILE:pal> DESTINATION "test/platform" OPTIONAL)
endif ()
