# xlang

This repo is the starting point for the xlang project, which enables developers to take existing shared libraries,
implemented in one programming language and make that library's APIs available to client code using a different programming language. Thus the name "xlang", for cross-language.  

See these related repos:

|Repository|Status|
|-|-|
|[C++/WinRT Language Projection](https://github.com/microsoft/cppwinrt)|[![Build status](https://dev.azure.com/microsoft/Dart/_apis/build/status/cppwinrt%20internal%20build)](https://dev.azure.com/microsoft/Dart/_build/latest?definitionId=31784)|
|[WinMD Parser Library](https://github.com/microsoft/winmd)|[![Build Status](https://dev.azure.com/microsoft/Dart/_apis/build/status/WinMD%20Nuget?branchName=master)](https://dev.azure.com/microsoft/Dart/_build/latest?definitionId=44715&branchName=master)|
[WinRT Test Component](https://github.com/microsoft/TestWinRT)||

Additionally, the xlang toolset will be available on multiple operating systems.
This means that if your shared library is portable to various operating systems,
then you can use the xlang tooling to make that shared library available to various client programming language on those various platforms.

More succinctly, you can take a library written in language A and make it available to language B applications running on platform C. The set of supported languages and platforms will expand as the project progresses.

The xlang project is in a very, very early stage of development.
The project wants and encourages community feedback and contributions. As such, the xlang team is doing *all* xlang project development in the open on GitHub. 

## What xlang is NOT

* The xlang project is not a port of the Windows Runtime, COM, DCOM or related technology.
* The xlang project will not port the Windows Runtime APIs.

## Project details

For details on project structure and build process, please see the [Project Readme](./src/readme.md).

For technical design details, please see the [Design Notes](./design_notes).

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

