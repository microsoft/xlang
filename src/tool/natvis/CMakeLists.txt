# The natvis cppwinrtvisualizer.dll is only targeted at Visual Studio (on Windows)
if (WIN32)

    # determine if VS SDK workload is installed via vswhere.exe 
    file(TO_NATIVE_PATH "$ENV{ProgramFiles\(x86\)}/Microsoft Visual Studio/Installer/vswhere.exe" vswhere_exe)
    if (EXISTS ${vswhere_exe})

        execute_process(
            COMMAND ${vswhere_exe} -requires Microsoft.VisualStudio.Workload.VisualStudioExtension -format json
            OUTPUT_VARIABLE vswhere_output
        )
        string(STRIP ${vswhere_output} vswhere_output)
        string(COMPARE NOTEQUAL ${vswhere_output} "[]" vssdk_installed)

        if (vssdk_installed)

            project(cppwinrtvisualizer)
            add_library(cppwinrtvisualizer SHARED)

            target_sources(cppwinrtvisualizer PUBLIC pch.cpp dllmain.cpp cppwinrt_visualizer.cpp object_visualizer.cpp property_visualizer.cpp cppwinrtvisualizer.def)
            TARGET_CONFIG_MSVC_PCH(cppwinrtvisualizer pch.cpp pch.h)
            target_include_directories(cppwinrtvisualizer PUBLIC ${XLANG_LIBRARY_PATH} "$ENV{VSInstallDir}DIA SDK/include" "$ENV{VSInstallDir}VSSDK/VisualStudioIntegration/Common/inc")
            # VSSDK headers not conformant
            string(APPEND CMAKE_CXX_FLAGS " /wd4471")

            target_link_libraries(cppwinrtvisualizer windowsapp shell32 advapi32 $ENV{VSInstallDir}VSSDK/VisualStudioIntegration/Common/lib/$ENV{VSCMD_ARG_TGT_ARCH}/vsdebugeng.lib)

        endif(vssdk_installed)
    endif(EXISTS ${vswhere_exe})
endif(WIN32)