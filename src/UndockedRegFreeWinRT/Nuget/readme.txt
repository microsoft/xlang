========================================================================
The Microsoft.Windows.UndockedRegFreeWinrt NuGet package
enabling you consume Windows Runtime classes registry free!
========================================================================

Installing this package will enable non-packaged desktop applications 
to leverage user-defined Windows Runtime types via the use of the 
funsion manifest.

Example fusion manifest:

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

This feature will disable itself on 19h1 and above version of windows.
However you must target your application for 
windows for it to self disable
https://docs.microsoft.com/en-us/windows/win32/sysinfo/targeting-your-application-at-windows-8-1

========================================================================
For more information, visit:
https://github.com/microsoft/xlang
========================================================================
