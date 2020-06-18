# Undocked RegFree WinRT

[![Build Status](https://dev.azure.com/microsoft/Dart/_apis/build/status/Undocked%20RegFree%20WinRT%20Nuget?branchName=master)](https://dev.azure.com/microsoft/Dart/_build/latest?definitionId=47851&branchName=master)

Enable non-packaged desktop applications 
to leverage user-defined Windows Runtime types via the use of the 
application manifests down to RS2. 

Example application manifest:
 ``` 
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<assembly xmlns="urn:schemas-microsoft-com:asm.v1" manifestVersion="1.0">
  <file name="TestComponent.dll">
    <activatableClass
        clsid="TestComponent.Class1"
        threadingModel="Both"
        xmlns="urn:schemas-microsoft-com:winrt.v1" />
    <activatableClass
        clsid="TestComponent.Class2"
        threadingModel="sta"
        xmlns="urn:schemas-microsoft-com:winrt.v1" />
    <activatableClass
        clsid="TestComponent.Class3"
        threadingModel="mta"
        xmlns="urn:schemas-microsoft-com:winrt.v1" />
  </file>
</assembly>
 ``` 

Starting with Windows 10 19h1, the operating system has this feature by default and will use this information directly. Be sure to use [Windows version targeting](https://docs.microsoft.com/en-us/windows/win32/sysinfo/targeting-your-application-at-windows-8-1) so that the package automatically disables itself when this feature is available by default.


### For more information about using application manifests for Windows Runtime types, visit:
[Enhancing Non-packaged Desktop Apps using Windows Runtime Components](https://blogs.windows.com/windowsdeveloper/2019/04/30/enhancing-non-packaged-desktop-apps-using-windows-runtime-components/)


