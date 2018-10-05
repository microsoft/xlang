# Xlang

The Xlang project contains the tools and components to support the cross-language and cross-platform runtime by Microsoft.

## Folder Structure

### /Library

The **/library** folder contains the C++ header libraries provided by Xlang for parsing and generating various formats.

* **cmd_reader.h** parses command line arguments.

* **meta_reader.h** parses [ECMA-335 metadata](http://www.ecma-international.org/publications/standards/Ecma-335.htm), similar to [Windows's metadata APIs](http://docs.microsoft.com/en-us/windows/desktop/api/rometadataapi/) but working directly against the metadata tables and written almost entirely in standard C++. 

* **task_group.h** executes tasks in parallel for release builds but one at a time in debug builds. 

* **text_writer.h** writes formatted text output.

### /Platform

The **/platform** folder contains the declaration and implementations of the common and minimal C API (and platform-specific implementations) used to support Xlang on different platforms (otherwise known as the PAL or Platform Adaptation Layer).

Eventually, we plan to break out the PAL into seperate folder based on underlying platform (Windows, Linux, Android, etc). For now, we only have Windows implementations.

* **published/pal.h** contains the delaration of the PAL surface area

### /Tool

The **/tool** folder contains the tools provided in support of Xlang development. This includes tools for working with idl and winmd files as well as for generating language projections.

* **/tool/cpp** implements the tool that generates the C++ 17 language projection. Currently, the generated projection is Windows only, but the tool will be updated to generate cross-plaform compatible code.

* **/tool/idl** implements a tool that generates [MIDL 3](http://docs.microsoft.com/en-us/uwp/midl-3/) files from ECMA-335 metadata.

* **/tool/natviz** implements a [Natvis visualization](http://docs.microsoft.com/en-us/visualstudio/debugger/create-custom-views-of-native-objects) of C++/WinRT componetns for the Visual Studio debugger 2017. Over time, this tool will be updated to support visualization of Xlang components.

* **/tool/python** implements the tool that generates the Python language projection. Currently, this tool is in pre-alpha state and builds on C++/WinRT projection from the Windows SDK rather than Xlang. The tool will be updated to generate cross-platform compatible code.

### /Test

The **/test** folder contains the unit tests for testing the libraries, platforms, and projections.

### /Scripts

The **/script** folder contains the scripts and tools used to build and bootstrap the various projects and language projections.

* **windowsBuild.ps1** builds Xlang for Windows. Note, this must be run from a VS 2017 command prompt. 

* **bionicBuild.cmd** builds Xlang for Ubuntu 18.04 (aka "Bionic Beaver") via Windows Subsystem for Linux.

* **wslConfigBionic.sh** bash script to configure Ubuntu 18.04 with the tools needed for Xlang.

* **scorch.cmake** cmake function to delete build artifacts not automatically removed by the clean target.

* **genStringLiteralFiles.cmake** cmake function to generate C++ string literals from files.

## Building

Xlang uses CMake instead of vcxproj in order to support cross platform builds. 

### Prerequisites

Xlang currently requires the following to be installed
* [Visual Studio 15.8.6](http://visualstudio.microsoft.com/downloads/) (or later)
    * Desktop Development with C++ workload
    * Visual C++ tools for CMake component
* [Windows SDK 17663](http://go.microsoft.com/fwlink/?LinkID=2023014) (or later)


### Known Issues

The following build failures indicate using outdated tools:

> src\platform\string_base.h(74): error C2327: 'xlang::impl::string_storage_base::alternate_form': is not a type name, static, or enumerator

    Solution: upgrade to Visual Studio 15.8.6 or later

> C:\Program Files (x86)\Windows Kits\10\include\10.0.17134.0\cppwinrt\winrt\base.h(2185): error C3861: 'from_abi': identifier not found

    Solution: upgrade to Windows SDK 17663 or later


### Building with Visual Studio 2017

Visual Studio 2017 has built in CMake support. For best results, use the latest update.

1. Select File -> Open -> CMake... from the main menu, navigate to the root of this repo and select the CMakeLists.txt file.
2. Wait a bit for VS to parse the CMake files. 
    * the src/CMakeSettings.json file is configured to generate the build files into the _build folder of the repo. Under the _build folder, build files and artifacts are generated under the *Platform*/*Architecture*/*Configuration* subfolder (for example, _build/Windows/x86/Debug)
3. To build, you can do any of the following
    * Select CMake -> Build All to build everything
    * Select CMake -> Build Only -> *Build Target* to build a specific target and its depenencies. 
    * Select a target executable from the Startup Item dropdown (with the green arrow) and use Visual Studio's build and debug hotkeys (Ctrl-Shift-B to build, F5 to build and debug, Shift-F5 to build and run)

### Command Line

Building Xlang from the command line is a two step process - you must generate the build files using [CMake](http://cmake.org/) and then build via [Ninja](http://ninja-build.org). Both of these tools are installed as part of the Visual C++ tools for CMake component. The build scripts described above automate the process of running CMake and they generate  build files and artifacts into the same folder structure Visual Studio uses (described above).

Command line parameters (both windowsBuild.ps1 and bionicBuild.ps1)
* -target **target name** specifies the target to build. Defaults to build all if not specified.
* -buildType **build type** specifies the build type (one of CMake's four standard build types: Debug, Release, RelWithDebInfo or MinSizeRel). Defaults to Debug if not specified.
* -forceCMake switch forces script to rerun cmake. Typically, Cmake is only run if CMakeCache.txt doesn't exist in the build folder.
* $verbose switch runs Ninja build step in verbose mode


