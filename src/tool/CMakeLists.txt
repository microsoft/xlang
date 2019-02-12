add_subdirectory(abi)
add_subdirectory(cpp)
add_subdirectory(idl)
add_subdirectory(natvis)
add_subdirectory(python)

if (WIN32)
    add_subdirectory(foo)
endif()