# Undocked RegFree WinRT

[![Build Status](https://dev.azure.com/microsoft/Dart/_apis/build/status/Undocked%20RegFree%20WinRT%20Nuget?branchName=master)](https://dev.azure.com/microsoft/Dart/_build/latest?definitionId=47851&branchName=master)

Enable non-packaged desktop applications 
to leverage user-defined Windows Runtime types via the use of the 
application manifests down to RS2. 

Example application manifest:
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

Since the regfree winrt feature is already available on versions of windows 19h1 and above, 
this package will automatically disable itself on 19h1 and above.  
However you must target your application for windows for it to self-disable.
[Targeting your application for Windows](https://docs.microsoft.com/en-us/windows/win32/sysinfo/targeting-your-application-at-windows-8-1)

### For more information about using application manifests for Windows Runtime types, visit:
[Enhancing Non-packaged Desktop Apps using Windows Runtime Components](https://blogs.windows.com/windowsdeveloper/2019/04/30/enhancing-non-packaged-desktop-apps-using-windows-runtime-components/)


