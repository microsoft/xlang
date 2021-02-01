## What is xlang?

The xlang project is the hub for the constellation of tools that enable development of Windows applications across a variety of programming languages. This includes tooling to process metadata, and tooling to access APIs from various programming languages including C#, C++, Rust, and Python.

Code in this repository provides infrastructure and tooling related to basic metadata handling & app usage. Tooling for each language is (mostly) maintained in separate repositories. Issue and work tracking for each tool or language is tracked in the corresponding repository as well. Issues that span languages or relate to basic metadata issues are tracked here.

### Metadata & Infrastructure

* Undocked RegFree WinRT (this repo)
* WinRT ABI Header Generation Tool (this repo)
* [WinRT test component](https://github.com/microsoft/TestWinRT) - Provides a compact but thorough test suite for validating projection support for consuming and implementing Windows Runtime-style APIs

### C++

* [C++/WinRT](https://github.com/microsoft/cppwinrt)
* [C++ winmd parser](https://github.com/microsoft/winmd)

### C#

* [C#/WinRT](https://github.com/microsoft/cswinrt)


### Rust

* [Rust/WinRT](https://github.com/microsoft/winrt-rs)

### Python

* Python/WinRT (experimental, this repo)

### Cross-platform WinRT

When this repository first created, we used this space to explore what it would take to bring Windows Runtime APIs to other platforms. While that investment has been put on hold for the time being, some of the projects and samples in this repo remain for but may be in disrepair.

## Related projects

* [Project Reunion](https://github.com/microsoft/projectreunion)

* [.NET 5](https://github.com/dotnet)


## License

Code licensed under the [MIT License](LICENSE).

## Contributing

This project welcomes contributions and suggestions.  Most contributions require you to agree to a
Contributor License Agreement (CLA) declaring that you have the right to, and actually do, grant us
the rights to use your contribution. For details, visit https://cla.microsoft.com.

When you submit a pull request, a CLA-bot will automatically determine whether you need to provide
a CLA and decorate the PR appropriately (e.g., label, comment). Simply follow the instructions
provided by the bot. You will only need to do this once across all repos using our CLA.

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/).
For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or
contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.

