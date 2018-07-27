@echo off
setlocal

set ROOT_PATH=%~dp0..
set BUILD_PATH=%ROOT_PATH%\_msbuild\%VSCMD_ARG_TGT_ARCH%

FOR /F "tokens=* USEBACKQ" %%F IN (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath`) DO (
    SET CMAKEPATH="%%F\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
)

%CMAKEPATH% "%ROOT_PATH%" "-B%BUILD_PATH%"
