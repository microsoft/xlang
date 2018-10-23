cmake_minimum_required(VERSION 3.9)

set(XLANG_SCRIPTS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/scripts")
set(XLANG_LIBRARY_PATH "${CMAKE_CURRENT_SOURCE_DIR}/library")

function(TARGET_CONFIG_MSVC_PCH target pch_cpp pch_header)

    set(pch_output_file "\"${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pch\"")

    get_target_property(sources ${target} SOURCES)
    foreach(file ${sources})
        if (${file} STREQUAL ${pch_cpp})
            set_source_files_properties(${file}
                PROPERTIES
                COMPILE_FLAGS " /Yc${pch_header} /Fp${pch_output_file}"
                OBJECT_OUTPUTS ${PROJECT_NAME}.pch)
        else()
            set_source_files_properties(${file}
                PROPERTIES
                COMPILE_FLAGS " /Yu${pch_header} /Fp${pch_output_file}"
                OBJECT_DEPENDS ${PROJECT_NAME}.pch)
        endif()
    endforeach()

    set_target_properties(${target} PROPERTIES PCH_OUTPUT_FILE ${pch_output_file})
    set_property(GLOBAL APPEND PROPERTY PCH_OUTPUT_FILES ${pch_output_file})

endfunction(TARGET_CONFIG_MSVC_PCH)

function(GENERATE_STRING_LITERAL_FILES glob_expression output_filename namespace target)

    file(GLOB input_files "${glob_expression}")

    set(out_h   "${PROJECT_BINARY_DIR}/${output_filename}.h")
    set(out_cpp "${PROJECT_BINARY_DIR}/${output_filename}.cpp")
    set(output_files ${out_h} ${out_cpp})

    add_custom_command(
        OUTPUT  ${output_files}
        DEPENDS ${input_files}
        COMMAND ${CMAKE_COMMAND} \"-Dglob_expression=${glob_expression}\" \"-Dout_h=${out_h}\" \"-Dout_cpp=${out_cpp}\" \"-Dnamespace=${namespace}\" -P \"${XLANG_SCRIPTS_PATH}/genStringLiteralFiles.cmake\"
    )

    set(gen_target_name "generated_${target}_${output_filename}")
    add_custom_target(${gen_target_name} DEPENDS ${output_files})
    add_dependencies(${target} ${gen_target_name})

endfunction()

function(CONSUME_PAL target)
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:pal>
        $<TARGET_FILE_DIR:${target}>
    )

    target_link_libraries(${target} pal)
endfunction()


add_custom_target(scorch COMMAND ${CMAKE_COMMAND} \"-Dfolder=${CMAKE_CURRENT_BINARY_DIR}\" -P \"${XLANG_SCRIPTS_PATH}/scorch.cmake\")
set_target_properties(scorch PROPERTIES EXCLUDE_FROM_ALL TRUE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    # change the warning level to 4
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

    # change from dynamic to static CRT
    string(REPLACE "/MDd" "/MTd" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    foreach(build_type RELEASE MINSIZEREL RELWITHDEBINFO)
        string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_${build_type} "${CMAKE_CXX_FLAGS_${build_type}}")
        string(APPEND CMAKE_CXX_FLAGS_${build_type} " /GL")

        string(REPLACE "/INCREMENTAL" "" CMAKE_EXE_LINKER_FLAGS_${build_type} "${CMAKE_EXE_LINKER_FLAGS_${build_type}}")
        string(APPEND CMAKE_EXE_LINKER_FLAGS_${build_type} " /LTCG:INCREMENTAL /OPT:REF")
    endforeach()
    
    add_compile_options(/await)
    add_compile_options(/permissive-)
    string(APPEND CMAKE_CXX_FLAGS_DEBUG " /JMC")

    # Explicitly configure _DEBUG preprocessor macro 
    string(APPEND CMAKE_CXX_FLAGS_DEBUG " /D_DEBUG")
else()
    add_compile_options(-stdlib=libc++)
    add_compile_options(-fcoroutines-ts)

    # Add _DEBUG preprocessor macro for compatibility with MSVC
    string(APPEND CMAKE_CXX_FLAGS_DEBUG " -D_DEBUG")
endif()

if (WIN32)
    add_definitions(-DNOMINMAX)
endif()

add_subdirectory(tool)
add_subdirectory(test)

# TODO: enable platform compiling on Ubuntu
if (WIN32)
add_subdirectory(platform)
endif()