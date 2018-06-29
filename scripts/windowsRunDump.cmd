@echo off
setlocal

set ROOT_PATH=%~dp0..
set BUILD_PATH=%ROOT_PATH%\_build\windows\dbg
"%BUILD_PATH%\tool\dump\dump.exe"