set(WINRT_INCLUDE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/include")

if(NOT DEFINED REGFREE_WINRT_BUILD_VERSION)
set(REGFREE_WINRT_BUILD_VERSION "1.0.0")
endif()

if (WIN32)
add_subdirectory(activation)
add_subdirectory(detours)
add_subdirectory(example)
add_subdirectory(test)
endif()

file(TO_NATIVE_PATH "${CMAKE_BINARY_DIR}/build_tools/nuget.exe" nuget_exe)
file(TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}" regfree_winrt_nupkg_dir)
file(TO_NATIVE_PATH "${regfree_winrt_nupkg_dir}/Microsoft.Windows.RegFreeWinRT.nupkg" regfree_winrt_nupkg)

file(DOWNLOAD https://dist.nuget.org/win-x86-commandline/latest/nuget.exe ${nuget_exe})

add_custom_command(OUTPUT ${regfree_winrt_nupkg}
    COMMAND ${nuget_exe} pack ${CMAKE_CURRENT_SOURCE_DIR}/Microsoft.Windows.UndockedRegFreeWinrt.nuspec -Properties winrtactinclude=${CMAKE_CURRENT_SOURCE_DIR}/include/extwinrt.h -Version ${REGFREE_WINRT_BUILD_VERSION} -OutputDirectory ${CMAKE_CURRENT_BINARY_DIR} -NonInteractive -Verbosity Detailed
)

#"C:\Users\kythant\Downloads\nuget.exe" pack "E:\xlang\src\winrt\Microsoft.Windows.RegFreeWinrt.nuspec" -Properties winrtactinclude="C:\Users\kyawt\source\repos\xlang\src\winrt\include\extwinrt.h" -NonInteractive -Verbosity Detailed
