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

set(LW_LIBRARY_PATH "${CMAKE_CURRENT_SOURCE_DIR}/library")

set(CMAKE_CXX_STANDARD 17)

if (WIN32)
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
    add_compile_options(-stdlib=libc++)
    add_compile_options(-pthread)
    add_compile_options(-fexceptions)
    add_compile_options(-fPIC)
    add_compile_options(-g -gdwarf-4)  

    SET( CMAKE_SHARED_LINKER_FLAGS  "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined" ) 
endif()

add_subdirectory(tool)
