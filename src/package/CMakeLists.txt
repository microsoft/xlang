cmake_minimum_required(VERSION 3.9)

file(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../tool/cpp/cppwinrt/version.h cppwinrt_version_h)
file(READ ${cppwinrt_version_h} cppwinrt_version_text)
string(REGEX MATCH ".*\"(.*)\".*" _ ${cppwinrt_version_text})
set(cppwinrt_version ${CMAKE_MATCH_1})

add_subdirectory(nuget)
add_subdirectory(vsix)
