@echo off
powershell -ExecutionPolicy Unrestricted -NoLogo -NoProfile -File %~dp0\Build.ps1 %*
