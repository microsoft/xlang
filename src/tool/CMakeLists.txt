add_subdirectory(abi)
add_subdirectory(python)
add_subdirectory(cppxlang)
add_subdirectory(xmeta)

if (WIN32)
add_subdirectory(cppwinrt)
endif()
