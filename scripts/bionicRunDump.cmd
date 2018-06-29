@echo off
setlocal

set WIN_ROOT_PATH=%~dp0..
set WIN_ROOT_PATH=%WIN_ROOT_PATH:\=\\%

FOR /F "tokens=* USEBACKQ" %%F IN (`ubuntu1804 run wslpath "%WIN_ROOT_PATH%"`) DO (
    SET ROOT_PATH=%%F
)

set BUILD_PATH=%ROOT_PATH%/_build/bionic/dbg

ubuntu1804 run "%BUILD_PATH%/tool/dump/dump"