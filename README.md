[![Build Status](https://microsoft.visualstudio.com/Dart/_apis/build/status/Xlang%20GitHub%20Daily%20Build)](https://microsoft.visualstudio.com/Dart/_build/latest?definitionId=31784)

# Xlang

## Overview
The xlang project enables developers to take existing shared libraries,
implemented in one programming language,
and make that library's APIs available to client code using a different programming language.
Thus the name "xlang," for cross-language.

Additionally, all of the xlang tooling will be platform-agnostic therefore will work on your favorite operating system.
This means that if your shared library is platform-agnostic (i.e. portable to various operating systems),
then you can use the xlang tooling to make that shared library available to various client programming language on those various platforms.

A *hypothetical* example might be taking the Python numpy library and making it available to C# applications running on Linux.

We're a *long* way was achieving that ultimate goal. We are in the very, very early stages of starting the project.
However, we want and encourage community feedback and contributions. As such, we decided to do *all* of the project development in the open on GitHub.
We look forward to your thoughts on the project.

## What xlang is NOT

* The xlang project is not a port of the Windows Runtime, COM, DCOM or related technology.
  * That said, many of the Microsoft members of the xlang team designed the Windows Runtime. As a result, we hope to incorporate many of the learnings from that prior project.
* The xlang project will not port any of the Windows Runtime APIs.
  * In fact, the xlang project is not delivering *any* shared libraries as part of this effort. We will leave the selection of interesting libraries that provide xlang support to the community.

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
