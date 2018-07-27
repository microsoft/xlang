cmake_minimum_required(VERSION 3.5)

message(FATAL_ERROR "platform adaptation layer not building for langworthy yet")

# if (WIN32)
#     message(SEND_ERROR "platform adaptation layer not needed for Windows")
# endif()

# set(sources
#     runtime.cpp
#     strings.cpp)

# add_library(WinRT.PAL.Static STATIC ${sources})
# add_library(WinRT.PAL.Shared SHARED ${sources})

# include_directories(../library)

# set_target_properties(WinRT.PAL.Shared PROPERTIES OUTPUT_NAME WinRT_PAL)
# set_target_properties(WinRT.PAL.Static PROPERTIES OUTPUT_NAME WinRT_PAL_static)

# if (NOT WIN32)
#     target_link_libraries(WinRT.PAL.Shared c++)
# endif()