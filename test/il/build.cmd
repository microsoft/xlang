md ..\output

ilasm foundation.il /MDV="WindowsRuntime 1.2" /dll /output=..\output\foundation.winmd
ilasm simple.il /MDV="WindowsRuntime 1.2" /dll /output=..\output\simple.winmd
ilasm component.il /MDV="WindowsRuntime 1.2" /dll /output=..\output\component.winmd

C:\git\Langworthy\_build\Windows\x86\Debug\tool\cpp\cpp.exe -in ..\output\foundation.winmd -output ..\output\foundation -verbose
C:\git\Langworthy\_build\Windows\x86\Debug\tool\cpp\cpp.exe -in ..\output\simple.winmd -ref ..\output\foundation.winmd -output ..\output\simple -verbose
C:\git\Langworthy\_build\Windows\x86\Debug\tool\cpp\cpp.exe -in ..\output\component.winmd -ref ..\output\foundation.winmd -output ..\output\component\source -component ..\output\component\gen -verbose
