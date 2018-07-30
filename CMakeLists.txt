cmake_minimum_required(VERSION 3.9)

function(TARGET_CONFIG_MSVC_PCH target pch_cpp pch_header)
    GET_TARGET_PROPERTY(sources ${target} SOURCES)
    foreach(file ${sources})
        if (${file} STREQUAL ${pch_cpp})
            SET_SOURCE_FILES_PROPERTIES(${file}
                PROPERTIES
                COMPILE_FLAGS " /Yc${pch_header} /Fp\"${CMAKE_CURRENT_BINARY_DIR}\\${PROJECT_NAME}.pch\""
                OBJECT_OUTPUTS ${PROJECT_NAME}.pch)
        else()
            SET_SOURCE_FILES_PROPERTIES(${file}
                PROPERTIES
                COMPILE_FLAGS " /Yu${pch_header} /Fp\"${CMAKE_CURRENT_BINARY_DIR}\\${PROJECT_NAME}.pch\""
                OBJECT_DEPENDS ${PROJECT_NAME}.pch)
        endif()
    endforeach()
endfunction(TARGET_CONFIG_MSVC_PCH)

set(XLANG_LIBRARY_PATH "${CMAKE_CURRENT_SOURCE_DIR}/library")

if (WIN32)
    add_compile_options(/std:c++17)

    # change the warning level to 4
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

    # change from dynamic to static CRT
    string(REPLACE "/MDd" "/MTd" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    foreach(build_type RELEASE MINSIZEREL RELWITHDEBINFO)
        string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_${build_type} "${CMAKE_CXX_FLAGS_${build_type}}")
    endforeach()
    
    # Add DEBUG preprocessor macro 
    string(APPEND CMAKE_CXX_FLAGS_DEBUG " /DDEBUG")

    add_compile_options(/permissive-)
else()
    add_compile_options(-std=c++17)
    add_compile_options(-stdlib=libc++)

    # Add DEBUG preprocessor macro 
    string(APPEND CMAKE_CXX_FLAGS_DEBUG " -DDEBUG")
endif()

add_subdirectory(tool)
