project(test_comp)

set(sources test_comp.cpp module.cpp exports.def)

add_library(test_comp SHARED ${sources})

target_link_libraries(test_comp windowsapp ole32)
target_include_directories(test_comp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})