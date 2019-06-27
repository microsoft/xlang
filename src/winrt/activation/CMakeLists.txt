project(winrtact)

set(sources demo.cpp catalog.cpp)

add_executable(winrtact "")
target_sources(winrtact PUBLIC ${sources})

target_include_directories(winrtact PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(winrtact PROPERTIES OUTPUT_NAME "winrtact")
target_include_directories(winrtact PUBLIC ${DETOURS_LIBRARY_PATH} ${XLANG_LIBRARY_PATH} ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR})
target_link_libraries(winrtact windowsapp detours)

add_dependencies(winrtact detours)


