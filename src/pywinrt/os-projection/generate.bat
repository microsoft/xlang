@echo on
:: Generate the Windows Runtime Projection DLL
:: Usage
::   generate.bat <pywinrt folder> <cppwinrt folder> <output path>

set windows_sdk=10.0.19041.0

set pywinrt_path="%~1"
set cppwinrt_path="%~2"
set output_path="%~3"

set pywinrt_exe=%pywinrt_path%\pywinrt.exe
set cppwinrt_exe=%cppwinrt_path%\cppwinrt.exe

:: clean output folder
:: TODO: fake version...
echo erase /s /q %output_path%\*

:: TODO: swap this later once basically working.
:: set %namespaces%="Windows."
set namespaces=-include Windows.Data.Json -include Windows.Devices.Geolocation -include Windows.Foundation -include Windows.Graphics.DirectX
set exclude=-exclude Windows.UI.Composition -exclude Windows.UI.Xaml


echo %cppwinrt_exe% -input %windows_sdk% -output %output_path%\cppwinrt -verbose
%cppwinrt_exe% -input %windows_sdk% -output %output_path%\cppwinrt -verbose
echo %pywinrt_exe% -input %windows_sdk% %namespaces% %exclude% -output %output_path% -verbose 
%pywinrt_exe% -input %windows_sdk% %namespaces% %exclude% -output %output_path%\pywinrt -verbose 
