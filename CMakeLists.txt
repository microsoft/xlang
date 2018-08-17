cmake_minimum_required(VERSION 3.9)

set(XLANG_SCRIPTS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/scripts")
set(XLANG_LIBRARY_PATH "${CMAKE_CURRENT_SOURCE_DIR}/library")

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

function(GENERATE_STRING_LITERAL_FILES glob_expression output_filename namespace target)

    file(GLOB input_files "${glob_expression}")

    set(out_h   "${PROJECT_BINARY_DIR}/${output_filename}.h")
    set(out_cpp "${PROJECT_BINARY_DIR}/${output_filename}.cpp")
    set(output_files ${out_h} ${out_cpp})

    add_custom_command(
        OUTPUT  ${output_files}
        DEPENDS ${input_files}
        COMMAND
            ${CMAKE_COMMAND} \"-Dglob_expression=${glob_expression}\" \"-Dout_h=${out_h}\" \"-Dout_cpp=${out_cpp}\" \"-Dnamespace=${namespace}\" -P \"${XLANG_SCRIPTS_PATH}/genStringLiteralFiles.cmake\"
    )

    set(gen_target_name "generated_${target}_${output_filename}")
    add_custom_target(${gen_target_name} DEPENDS ${output_files})
    add_dependencies(${target} ${gen_target_name})

endfunction()


if (WIN32)
    add_compile_options(/std:c++17)

    # change the warning level to 4
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

    # change from dynamic to static CRT
    string(REPLACE "/MDd" "/MTd" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    foreach(build_type RELEASE MINSIZEREL RELWITHDEBINFO)
        string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_${build_type} "${CMAKE_CXX_FLAGS_${build_type}}")
    endforeach()
    
    add_compile_options(/await)
    add_compile_options(/permissive-)

    # Explicitly configure _DEBUG preprocessor macro 
    string(APPEND CMAKE_CXX_FLAGS_DEBUG " /D_DEBUG")
else()
    add_compile_options(-std=c++17)
    add_compile_options(-stdlib=libc++)
    add_compile_options(-fcoroutines-ts)

    # Add _DEBUG preprocessor macro for compatibility with MSVC
    string(APPEND CMAKE_CXX_FLAGS_DEBUG " -D_DEBUG")
endif()

add_subdirectory(tool)
