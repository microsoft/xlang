project(detours)

set(sources detours.cpp modules.cpp disasm.cpp image.cpp creatwth.cpp
            disolx86.cpp disolx64.cpp disolia64.cpp disolarm.cpp disolarm64.cpp)

add_library(detours STATIC ${sources})

target_include_directories(detours PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(detours PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
set_target_properties(detours PROPERTIES OUTPUT_NAME "detours")

install(TARGETS detours DESTINATION "test/platform")

