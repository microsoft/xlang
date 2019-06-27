This is a very rough cut of a concept using the detours library to extend support for WinRT
components to desktop apps running any version of Windows down to Windows 8. This is meant
to prove out the concept, but there are a number of open challenges including the .NET 
support matrix for WinRT types. This is less problematic for code compiled as native code,
including components written in C++ /WinRT & C++ /CX so even if .NET support is incomplete,
some people may find this approach useful.

This is strictly an experiment at this point, and should not in any way be construed as the
start of something that will ever be complete or supported.

Thanks,

Ben Kuhn
