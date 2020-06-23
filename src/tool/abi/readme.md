## The ABI/WinRT language projection

The Application Binary Interface (ABI) projection generates header files for consuming WinRT APIs that are more familiar to headers generated from COM .idl files.

**Note: This tool is for legacy codebases and not recommended for new development. C++ developers should instead use https://github.com/microsoft/cppwinrt, which supports standard c++ and the latest c++ language features.**

- Nuget Package: https://www.nuget.org/packages/Microsoft.Windows.AbiWinRT/

## Usage

Similar to the `Microsoft.Windows.CppWinRT`nuget package, you can add a reference to `Microsoft.Windows.AbiWinRT` and the tool will generate the projection headers for your project to consume.

|  Property |  Description |
|-----------|---------------|
| `AbiWinRTModernIDL` | Passes the /winrt flag to midlrt to speed up compilation times. See https://docs.microsoft.com/en-us/uwp/midl-3/ for more information. Defaults to `false`. |
| `AbiWinRTAddAbiNamespacePrefix` | Adds the `ABI::` prefix to all generated headers. Defaults to `true`. |
| `AbiWinRTEnableComponentProjection` | Generates the projection headers for the winmd file generated from this project |
| `AbiWinRTEnablePlatformProjection` | Generates the projection headers for the Windows SDK |
| `AbiWinRTEnableReferenceProjection` | Generates the projection headers for any referenced winmd files |
| `AbiWinRTEnsureSDKHeaderCompat` | Passes the `-lowercase-include-guard` switch to `abi.exe` to enable compat with SDK headers. When `AbiWinRTEnablePlatformProjection` is `false` this defaults to `true`, otherwise it defaults to `false`. |
