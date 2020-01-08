project(regfree_winrt_test)

set(sources test_apartment_activation.cpp pch.cpp main.cpp)

add_executable(regfree_winrt_test "")
target_sources(regfree_winrt_test PUBLIC ${sources})

target_include_directories(regfree_winrt_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(regfree_winrt_test PROPERTIES OUTPUT_NAME "regfree_winrt_test")
target_include_directories(regfree_winrt_test PUBLIC ${WINRT_INCLUDE_PATH} ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR})
target_link_libraries(regfree_winrt_test windowsapp winrtact)

add_dependencies(regfree_winrt_test winrtact)

add_custom_command(TARGET regfree_winrt_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_BINARY_DIR}/../activation/winrtact.dll
	${CMAKE_CURRENT_BINARY_DIR}
)

file(GLOB test_files
  "test_files/*"
)
file(COPY ${test_files} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
