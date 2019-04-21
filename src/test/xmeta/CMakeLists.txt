cmake_minimum_required(VERSION 3.9)

project(test_xmeta)

# using /MD flag for antlr4_runtime (for Visual C++ compilers only)
set(ANTLR4_WITH_STATIC_CRT ON)

include_directories(
	${CMAKE_SOURCE_DIR}/tool/xmeta/
    ${CMAKE_SOURCE_DIR}/tool/xmeta/cpp/models/	
)

add_executable(test_xmeta "")
target_link_libraries(test_xmeta "WindowsApp.lib")
target_link_libraries(test_xmeta "comsuppw.lib")

if (EXISTS "${CMAKE_SOURCE_DIR}/test/xmeta/example/midl3test")
add_custom_command(TARGET test_xmeta POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/test/xmeta/example/midl3test"
    $<TARGET_FILE_DIR:test_xmeta>/midl3test)
endif()

target_sources(test_xmeta
    PUBLIC pch.cpp testlexer.cpp testmidlcompat.cpp testxmetaemit.cpp)

target_include_directories(test_xmeta
    PUBLIC ${XLANG_LIBRARY_PATH} ${XLANG_TEST_INC_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../output/component/source)

target_link_libraries(test_xmeta xmeta_antlr)

if (MSVC)
    TARGET_CONFIG_MSVC_PCH(test_xmeta pch.cpp pch.h)
    target_link_libraries(test_xmeta windowsapp ole32)
endif()

set(xmeta_gen_generated
    ${CMAKE_SOURCE_DIR}/test/xmeta/xlang_test_listener.cpp
    ${CMAKE_SOURCE_DIR}/tool/xmeta/xmeta_emit.cpp
    ${CMAKE_SOURCE_DIR}/tool/xmeta/xlang_model_walker.cpp
 )

target_sources(test_xmeta PUBLIC main.cpp ${xmeta_gen_generated})

install(TARGETS test_xmeta DESTINATION "test/xmeta")
if (WIN32)
    install(FILES $<TARGET_PDB_FILE:test_xmeta> DESTINATION "test/xmeta" OPTIONAL)
endif ()

