@echo off
:: Generate the Windows Runtime Projection DLL
:: Usage
::   generate.bat <pywinrt folder> <cppwinrt folder> <output path>

set windows_sdk=10.0.19041.0

set pywinrt_exe="%~1\pywinrt.exe"
set cppwinrt_exe="%~2\cppwinrt.exe"
set cpp_output_path="%~3\cppwinrt"
set py_output_path="%~3\pywinrt"

:: clean output folder
echo cleaning old output
echo erase /s /q %cpp_output_path%\*
echo erase /s /q %py_output_path%\*

:: To create a tighter inner loop, use the minimal set of namespaces below.
:: set namespaces=-include Windows.
set namespaces=-include Windows.Data.Json -include Windows.Devices.Geolocation -include Windows.Foundation -include Windows.Graphics.DirectX
set exclude=-exclude Windows.UI.Composition -exclude Windows.UI.Xaml


echo %cppwinrt_exe% -input %windows_sdk% -output %cpp_output_path% -verbose
%cppwinrt_exe% -input %windows_sdk% -output %cpp_output_path% -verbose
echo %pywinrt_exe% -input %windows_sdk% %namespaces% %exclude% -output %py_output_path% -verbose 
%pywinrt_exe% -input %windows_sdk% %namespaces% %exclude% -output %py_output_path% -verbose 
