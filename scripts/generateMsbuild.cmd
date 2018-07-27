@echo off
setlocal

set ROOT_PATH=%~dp0..
set BUILD_PATH=%ROOT_PATH%\_msbuild

cmake "%ROOT_PATH%" "-B%BUILD_PATH%"