project(test_platform)

add_executable(test_platform "")
target_sources(test_platform
    PUBLIC pch.cpp memory.cpp string.cpp activation.cpp)

target_include_directories(test_platform PUBLIC 
    ${XLANG_LIBRARY_PATH} 
    ${VCPKG_INCLUDE_PATH} 
    ${CMAKE_CURRENT_SOURCE_DIR}/../output/component/source)

CONSUME_PAL(test_platform)

if (MSVC)
    TARGET_CONFIG_MSVC_PCH(test_platform pch.cpp pch.h)
    target_link_libraries(test_platform windowsapp ole32)
else()
    target_link_libraries(test_platform c++ c++abi c++experimental)
    target_link_libraries(test_platform -lpthread)

    # set test_platform linker flags to look for .so files in the current directory 
    # adapted from https://serverfault.com/a/926072
    get_target_property(temp_link_flags test_platform LINK_FLAGS)
    if (temp_link_flags MATCHES "NOTFOUND$")
        set_target_properties(test_platform PROPERTIES LINK_FLAGS "-Wl,-rpath='$ORIGIN'")
    else()
        set_target_properties(test_platform PROPERTIES LINK_FLAGS "${temp_link_flags} -Wl,-rpath='$ORIGIN'")
    endif()
endif()

target_sources(test_platform PUBLIC main.cpp)

install(TARGETS test_platform DESTINATION "test/platform")
if (WIN32)
    install(FILES $<TARGET_PDB_FILE:test_platform> DESTINATION "test/platform" OPTIONAL)
endif ()
