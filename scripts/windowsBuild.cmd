@echo off
setlocal

set ROOT_PATH=%~dp0..
set BUILD_PATH=%ROOT_PATH%\_build\windows\dbg

cmake "%ROOT_PATH%" "-B%BUILD_PATH%" -GNinja -DCMAKE_BUILD_TYPE=Debug
cmake --build "%BUILD_PATH%"