cmake_minimum_required(VERSION 3.9)

project(xmeta)

if (WIN32)
    add_executable(xmeta
        main.cpp
    )
endif ()

if (WIN32)
    add_library(xmeta_model
        cpp/xmeta_idl_reader.cpp
        cpp/ast_to_st_listener.cpp
        cpp/models/namespace_model.cpp
        cpp/xlang_model_walker.cpp)
endif ()

target_include_directories(xmeta_model PUBLIC cpp/models cpp)

if (WIN32)
    add_library(xmeta_windows xmeta_emit.cpp)
    target_include_directories(xmeta_windows PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} cpp/models cpp ${XLANG_LIBRARY_PATH})
    target_link_libraries(xmeta_windows PUBLIC xmeta_model)
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/antlr4-runtime/cmake)

# using /MD flag for antlr4_runtime (for Visual C++ compilers only)
set(ANTLR4_WITH_STATIC_CRT ON)

# set variable pointing to the antlr tool that supports C++
# this is not required if the jar file can be found under PATH environment
set(ANTLR_EXECUTABLE ${CMAKE_CURRENT_SOURCE_DIR}/antlr4-runtime/antlr-4.7.2-complete.jar)
set(ANTLR4_ZIP_REPOSITORY ${CMAKE_CURRENT_SOURCE_DIR}/antlr4-runtime/antlr4-4.7.2.zip)

# add external build for antlrcpp
include(ExternalAntlr4Cpp)

set(antlr_inputs ${CMAKE_CURRENT_SOURCE_DIR}/grammar/XlangLexer.g4 ${CMAKE_CURRENT_SOURCE_DIR}/grammar/XlangParser.g4)
set(antlr_generated_dir ${CMAKE_CURRENT_BINARY_DIR}/antlr_generated)
set(antlr_generated_src
    ${antlr_generated_dir}/XlangLexer.cpp
    ${antlr_generated_dir}/XlangParser.cpp
    ${antlr_generated_dir}/XlangParserBaseListener.cpp
    ${antlr_generated_dir}/XlangParserListener.cpp
)

if (WIN32)
    add_library(xmeta_antlr)
endif ()

add_custom_command(OUTPUT ${antlr_generated_src}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${antlr_generated_dir}
    COMMAND ${ANTLR_EXECUTABLE} -o ${antlr_generated_dir} -Dlanguage=Cpp ${antlr_inputs}
    DEPENDS ${antlr_inputs})

target_sources(xmeta_antlr PRIVATE ${antlr_generated_src})
target_include_directories(xmeta_antlr PUBLIC
    ${antlr_generated_dir}
    ${ANTLR4_INCLUDE_DIRS} # antlr4cpp artifacts
)

target_link_libraries(xmeta_antlr PUBLIC antlr4_static)

# required if linking to static library
target_compile_definitions(xmeta_antlr PUBLIC -DANTLR4CPP_STATIC)

if (WIN32)
    target_link_libraries(xmeta_model PUBLIC xmeta_antlr)
endif ()

if (MSVC)
    target_compile_definitions(xmeta_antlr PUBLIC -D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)
endif ()

if (WIN32)
    target_link_libraries(xmeta
        xmeta_antlr
        xmeta_model
        xmeta_windows
        WindowsApp.lib
        comsuppw.lib
    )
endif()
