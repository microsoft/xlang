cmake_minimum_required(VERSION 3.12)

message("CMAKE_CURRENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(/await /bigobj /GL /GR- /permissive-)

project(_winrt)

if ("$ENV{VSCMD_ARG_TGT_ARCH}" STREQUAL "x64")
    set(PYTHON_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/python")
elseif("$ENV{VSCMD_ARG_TGT_ARCH}" STREQUAL "x86")
    set(PYTHON_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/pythonx86")
endif()

link_directories("${PYTHON_ROOT}/tools/libs")

file(GLOB sources "${CMAKE_CURRENT_SOURCE_DIR}/pywinrt/winrt/src/*.cpp")
add_library(_winrt MODULE ${sources})
set_target_properties(_winrt PROPERTIES SUFFIX ".pyd")
target_include_directories(_winrt PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/cppwinrt" "${PYTHON_ROOT}/tools/include")
target_link_libraries(_winrt PRIVATE onecore)

string(REGEX REPLACE "/GR" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
string(APPEND CMAKE_MODULE_LINKER_FLAGS " /LTCG")

if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "19.20.27508.0")
    string(APPEND CMAKE_CXX_FLAGS " /d2FH4")
endif()
