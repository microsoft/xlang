
project(winrtact)

set(sources dllmain.cpp catalog.cpp winrtact.def)

add_library(winrtact SHARED ${sources})

target_include_directories(winrtact PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(winrtact PROPERTIES OUTPUT_NAME "winrtact")
set_target_properties(winrtact PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(winrtact PUBLIC ${WINRT_INCLUDE_PATH} ${DETOURS_LIBRARY_PATH} ${XLANG_LIBRARY_PATH} ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR})
target_link_libraries(winrtact windowsapp detours)

add_dependencies(winrtact detours)
