Notes on test code:

TestApp vs UnitTest
dotnet xUnit tests provide a convenient way to validate (and fail) builds via XunitXml.TestLogger xml output.
Unfortunately, 'dotnet test' doesn't support appx manifest activatableClass entries for bulit-in C# projection.
OTOH, dotnet apps support project references for built-in C# WinRT projection support (for perf comparisons).
TODO: could generate a test xml report from TestApp to validate perf.

Tests use a native test component dll, so arch-specific configurations are required - AnyCPU is not supported.
In turn, both distributions of dotnet.exe (x86 and x64) are required for execution on each arch.
For TestApp:
	.NET Core 3.0 SDK - Windows x86 Installer (v3.0.100-preview6)
	.NET Core 3.0 SDK - Windows x64 Installer (v3.0.100-preview6)
For UnitTest:
	.NET Core 2.2 SDK - Windows x86 Installer (v2.2.301)
	.NET Core 2.2 SDK - Windows x64 Installer (v2.2.301)

Tests.sln is for inner dev loop - CmakeLists.txt builds and runs projects individually
	TestComp project is built via msbuild (dotnet doesn't support)
	UnitTest project is built and run via 'dotnet test', for convenience
	In VS, xUnit test arch must match machine's, or a System.BadImageFormatException is thrown
