project(activation_demo)

set(sources demo.cpp)

add_executable(activation_demo "")
target_sources(activation_demo PUBLIC ${sources})

target_include_directories(activation_demo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(activation_demo PROPERTIES OUTPUT_NAME "activation_demo")
target_include_directories(activation_demo PUBLIC ${WINRT_INCLUDE_PATH} ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR})
target_link_libraries(activation_demo windowsapp winrtact)

add_dependencies(activation_demo winrtact)