Notes on test code:

Tests use a native test component dll, so arch-specific configurations are required - AnyCPU is not supported.
activatableClass appx manifest entries (e.g., TestComp!TestComp.Class) require dotnet core 3.0 preview.  
In turn, both distributions of dotnet.exe (x86 and x64) are required for execution on each arch.
	.NET Core 3.0 SDK - Windows x86 Installer (v3.0.100-preview6)
	.NET Core 3.0 SDK - Windows x64 Installer (v3.0.100-preview6)

TestApp vs UnitTest
dotnet xUnit tests provide a convenient way to validate (and fail) builds via XunitXml.TestLogger xml output.
OTOH, only dotnet apps support project references for built-in C# WinRT projection support (for perf comparisons).
TODO: could generate a test xml report from TestApp to validate perf.

Tests.sln is for inner dev loop - CmakeLists.txt builds and runs projects individually
	TestComp project is built via msbuild (dotnet doesn't support)
	UnitTest project is built and run via 'dotnet test', for convenience
	In VS, xUnit test arch must match machine's, or a System.BadImageFormatException is thrown
