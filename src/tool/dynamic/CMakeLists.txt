find_library(FFI_LIBRARY_PATH libffi)
find_path(FFI_INCLUDE_DIR NAMES ffi.h)

project(dynamic)

add_executable(dynamic "")
target_sources(dynamic PUBLIC main.cpp pch.cpp sha1.cpp)
target_include_directories(dynamic PUBLIC 
    ${XLANG_LIBRARY_PATH} 
    ${FFI_INCLUDE_DIR} 
    ${PROJECT_SOURCE_DIR}/../cppwinrt/strings)
target_link_libraries(dynamic ${FFI_LIBRARY_PATH})

TARGET_CONFIG_MSVC_PCH(dynamic pch.cpp pch.h)
target_compile_options(dynamic PUBLIC /await)
target_link_libraries(dynamic windowsapp ole32)