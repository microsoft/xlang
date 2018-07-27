cmake_minimum_required(VERSION 3.9)

set(LW_LIBRARY_PATH "${CMAKE_CURRENT_SOURCE_DIR}/library")

# fix some MSVC defaults that CMake sets
if (MSVC)
    # add DEBUG Preprocessor Definition for debug builds
    string(APPEND CMAKE_CXX_FLAGS_DEBUG " /DDEBUG")

    # If warning level is set, change level to 4 
    if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
        string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    # If warning level is not set, add warning level 4
    else(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    endif(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")

    # change from dynamic to static CRT
    # https://gitlab.kitware.com/cmake/community/wikis/FAQ#how-can-i-build-my-msvc-application-with-a-static-runtime
    foreach(flags_var 
            CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
            CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
        if(${flags_var} MATCHES "/MD")
            string(REPLACE "/MD" "/MT" ${flags_var} "${${flags_var}}")
        endif(${flags_var} MATCHES "/MD")
    endforeach(flags_var)
endif()

set(CMAKE_CXX_STANDARD 17)

if (WIN32)
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
