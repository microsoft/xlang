@echo off
setlocal

set WIN_ROOT_PATH=%~dp0..
set WIN_ROOT_PATH=%WIN_ROOT_PATH:\=\\%

FOR /F "tokens=* USEBACKQ" %%F IN (`ubuntu1804 run wslpath "%WIN_ROOT_PATH%"`) DO (
    SET ROOT_PATH=%%F
)

set BUILD_PATH=%ROOT_PATH%/_build/bionic/dbg

ubuntu1804 run cmake "%ROOT_PATH%" "-B%BUILD_PATH%" -GNinja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang
ubuntu1804 run cmake --build "%BUILD_PATH%"