# xlang

The xlang project contains the tools and components to support the cross-language and cross-platform runtime by Microsoft.

## Building for Windows

xlang uses CMake instead of vcxproj in order to support cross platform builds.

### Prerequisites

Building xlang for Windows requires the following to be installed:

* [Visual Studio 2017](https://developer.microsoft.com/windows/downloads), version 15.8.7 or later.
  * Visual Studio's Desktop Development with C++ workload installation is required.
* [CMake](https://cmake.org/), version 3.9 or later
  * Visual Studio's Desktop Development with C++ workload includes CMake 3.11
* [Ninja](https://ninja-build.org/), version 1.8 or later
  * Visual Studio's Desktop Development with C++ workload includes Ninja 1.8.2

Building xlang for Linux requires the following to be installed. The [configureBionic.sh](/src/scripts/configureBionic.sh) script will automatically install these depenencies via [apt](https://en.wikipedia.org/wiki/APT_(Debian)).

* [Clang](http://clang.llvm.org/), version 6.0 or later.
* LLVM [libc++](http://libcxx.llvm.org/) and [libc++ ABI](http://libcxxabi.llvm.org/), release 60 or later.
* [CMake](https://cmake.org/), version 3.9 or later
* [Ninja](https://ninja-build.org/), version 1.8 or later

> Note, xlang has very few platform-specific dependencies, so it should be easy to port to most systems. However, it has only been tested with Ubuntu 18.04.

### Building for Windows with Visual Studio 2017

Visual Studio 2017 has built in CMake support. For best results, use Visual Studio's latest update.

1. Select File -> Open -> CMake... from the main menu, navigate to the root of your cloned xlang and select the CMakeLists.txt file.
2. Wait a bit for VS to parse the CMake files.
    * the src/CMakeSettings.json file is configured to generate the build files into the _build folder of the repo. Under the _build folder, build files and artifacts are generated under the *Platform*/*Architecture*/*Configuration* subfolder (for example, _build/Windows/x86/Debug)
3. To build, you can do any of the following
    * Select CMake -> Build All to build everything
    * Select CMake -> Build Only -> *Build Target* to build a specific target and its dependencies.
    * Select a target executable from the Startup Item dropdown (with the green arrow) and use Visual Studio's build and debug hotkeys (Ctrl-Shift-B to build, F5 to build and debug, Shift-F5 to build and run)

### Building for Windows on the Command Line

Building xlang (or any CMake based project) from the command line is a two step process - First the build files are generated using [CMake](http://cmake.org/) and then build is execute via [Ninja](http://ninja-build.org). Both of these tools are installed as part of the Visual C++ tools for CMake component.

The [windowsBuild.ps1](/src/scripts/windowsBuild.ps1) build script automates the process of running CMake and Ninja. It generates build files and artifacts into the same folder structure Visual Studio uses (described above). You must run windowsBuild.ps1 from a VS 2017 Developer command prompt. Open a Developer Command Prompt, navigate to the root of your cloned xlang repo and execute the following:

``` shell
powershell -noprofile -command "& {src\scripts\windowsBuild.ps1}"
```

windowsBuild.ps1 supports the following command line parameters:

* -target **target name** specifies the target to build. Defaults to build all if not specified.
* -buildType **build type** specifies the build type (one of CMake's four standard build types: Debug, Release, RelWithDebInfo or MinSizeRel). Defaults to Debug if not specified.
* -forceCMake switch forces script to rerun CMake. Typically, CMake is only run if CMakeCache.txt doesn't exist in the build folder.
* $verbose switch runs Ninja build step in verbose mode

### Building for Linux on the Command Line

> Note, xlang is a young project. While we have aspirations to support mutiple platforms (detailed in [XDN01](/design_notes/XDN01%20-%20A%20Strategy%20for%20Language%20Interoperability.md)), we are a long way from realizing that dream. In particular, while both the C++ and Python projection tools in the [/tools](/src/tools) directory can be compiled for Windows and Linux, today they both generate Windows specific code. As the xlang [Platform Adaptation Layer (aka PAL)](/src/platform) comes online, we will be updating all of our code to work with it, enabling us to target platforms beyond just Windows.

Building for Linux on the command line is similar to building for Windows on the command line. The [bionicBuild.sh](/src/scripts/bionicBuild.sh) script automates the process of running CMake and Ninja to produce a build. Similar to building on Windows, bionicBuild.sh generates build artifacts into the _build/*Platform*/*Configuration* folder. Note, only x64 builds are supported at this time, so the build script does not add an *Architecture* folder to the build path.

Command line arguments for bionicBuild.sh closely mirror the command line arguments for windowsBuild.ps1

* -t|--target **target name** specifies the target to build. Defaults to build all if not specified.
* -b|--build-type **build type** specifies the build type (one of CMake's four standard build types: Debug, Release, RelWithDebInfo or MinSizeRel). Defaults to Debug if not specified.
* -f|--force-cmake switch forces script to rerun cmake. Typically, Cmake is only run if CMakeCache.txt doesn't exist in the build folder.
* -v|--verbose switch runs Ninja build step in verbose mode

### Known Issues

The following build failures indicate using outdated tools:

> src\platform\string_base.h(74): error C2327: 'xlang::impl::string_storage_base::alternate_form': is not a type name, static, or enumerator

    Solution: upgrade to Visual Studio 15.8.6 or later

> C:\Program Files (x86)\Windows Kits\10\include\10.0.17134.0\cppwinrt\winrt\base.h(2185): error C3861: 'from_abi': identifier not found

    Solution: upgrade to Windows SDK 17663 or later

## Folder Structure

### /Library

The **/library** folder contains the C++ header libraries provided by xlang for parsing and generating various formats.

* **cmd_reader.h** parses command line arguments.

* **meta_reader.h** parses [ECMA-335 metadata](http://www.ecma-international.org/publications/standards/Ecma-335.htm), similar to [Windows's metadata APIs](http://docs.microsoft.com/en-us/windows/desktop/api/rometadataapi/) but working directly against the metadata tables and written almost entirely in standard C++. 

* **task_group.h** executes tasks in parallel for release builds but one at a time in debug builds. 

* **text_writer.h** writes formatted text output.

### /Platform

The **/platform** folder contains the declaration and implementations of the common and minimal C API (and platform-specific implementations) used to support xlang on different platforms (otherwise known as the PAL or Platform Adaptation Layer).

Eventually, source in the PAL will be split into separate folders based on underlying platform (Windows, Linux, Android, etc). For now, the PAL only has a Windows implementation.

* **published/pal.h** contains the declaration of the PAL surface area

### /Tool

The **/tool** folder contains the tools provided in support of xlang development. This includes tools for working with idl and winmd files as well as for generating language projections.

* **/tool/cpp** implements the tool that generates the C++ 17 language projection. Currently, the generated projection is Windows only, but the tool will be updated to generate cross-platform compatible code.

* **/tool/idl** implements a tool that generates [MIDL 3](http://docs.microsoft.com/en-us/uwp/midl-3/) files from ECMA-335 metadata.

* **/tool/natviz** implements a [Natvis visualization](http://docs.microsoft.com/en-us/visualstudio/debugger/create-custom-views-of-native-objects) of C++/WinRT components for the Visual Studio debugger 2017. Over time, this tool will be updated to support visualization of xlang components.

* **/tool/python** implements the tool that generates the Python language projection. Currently, this tool is in pre-alpha state and builds on C++/WinRT projection from the Windows SDK rather than xlang. The tool will be updated to generate cross-platform compatible code.

### /Test

The **/test** folder contains the unit tests for testing the libraries, platforms, and projections.

### /Scripts

The **/script** folder contains the scripts and tools used to build and bootstrap the various projects and language projections.

* **windowsBuild.ps1** builds xlang for Windows. Note, this must be run from a VS 2017 command prompt. 

* **bionicBuild.cmd** builds xlang for Ubuntu 18.04 (aka "Bionic Beaver") via Windows Subsystem for Linux.

* **wslConfigBionic.sh** bash script to configure Ubuntu 18.04 with the tools needed for xlang.

* **scorch.cmake** cmake function to delete build artifacts not automatically removed by the clean target.

* **genStringLiteralFiles.cmake** cmake function to generate C++ string literals from files.
