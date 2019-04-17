cmake_minimum_required(VERSION 3.12)

if(NOT DEFINED PACKAGE_ROOT_PATH)
    message(FATAL_ERROR "PACKAGE_ROOT_PATH not defined")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(/await /bigobj /GL /GR- /permissive-)

project(_winrt)

if ("$ENV{VSCMD_ARG_TGT_ARCH}" STREQUAL "x64")
    set(PYTHON_ROOT "${PACKAGE_ROOT_PATH}/tools/python")
elseif("$ENV{VSCMD_ARG_TGT_ARCH}" STREQUAL "x86")
    set(PYTHON_ROOT "${PACKAGE_ROOT_PATH}/tools/pythonx86")
endif()

link_directories("${PYTHON_ROOT}/tools/libs")

file(GLOB sources "${PACKAGE_ROOT_PATH}/pywinrt/winrt/src/*.cpp")
add_library(_winrt MODULE ${sources})
set_target_properties(_winrt PROPERTIES SUFFIX ".pyd")
target_include_directories(_winrt PUBLIC "${PACKAGE_ROOT_PATH}/cppwinrt" "${PYTHON_ROOT}/tools/include")
target_link_libraries(_winrt PRIVATE onecore)

string(REGEX REPLACE "/GR" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
string(APPEND CMAKE_MODULE_LINKER_FLAGS " /LTCG")

if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "19.20.27508.0")
    string(APPEND CMAKE_CXX_FLAGS " /d2FH4")
endif()
