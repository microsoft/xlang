# xlang

The xlang project contains the tools and components to support the cross-language and cross-platform runtime by Microsoft.

## Folder Structure
### Library

The **/library** folder contains the C++ libraries provided by xlang for parsing and generating various formats including winmd, idl, and text files.

**meta_reader.h** parses winmd files like C++/WinRT�s winmd_reader but reading the winmd tables directly rather than using the CLR APIs. The CLR APIs are wholly inadequate. This capability is essential no matter what we do or how we prioritize languages/platforms.

**meta_writer.h** writes winmd files like MIDLRT/MDMERGE but writes the winmd format directly rather than using the CLR APIs and ideally avoids the need for a distinct merge step. This is essential to unshackle this project from Microsoft proprietary compilers and tools, which are entirely at odds with the essence of this project.

**text_writer.h** writes formatted text output based on C++/WinRT�s text_writer with only minor improvements to make it agnostic of language output.

**idl_reader.h** parses IDL3 files. An IDL parser is essential to combine with the winmd writer to produce winmd files. While IDL continues to be a Microsoft proprietary language, it is a pragmatic choice until something like C++ metaclasses finally becomes widely available. 

**idl_writer.h** writes IDL3 files. While not directly required by developers it is essential for the complete roundtrip testing and is simply a byproduct of writing and testing winmd support. It will also be very useful in canonically expressing and storing (in GitHub) existing metadata (e.g. foundation types) for reference.

**winrt/base.h** The C++/WinRT base.h file from the prototype dev_xplat branch. Eventually, we will pull out the parts of base.h needed for xlang so we don't depend on any output of C++/WinRT.

### Platforms

The **/platform** folder contains the common and minimal C API (and platform-specific implementations) used to support language projections on different platforms, otherwise known as a PAL.

Eventually, we plan to break out the PAL into seperate folder based on underlying platform (Windows, Linux, Android, etc). For now, we only have the PAL from the prototype dev_xplat C++/WinRT branch.

### Tests

The **/test** folder contains the unit tests for testing the libraries, platforms, and projections.

### Tools

The **/tool** folder contains the tools provided in support of xlang development. This includes tools for working with idl and winmd files as well as for generating language projections.

**/tool/cpp** implements the tool that generates the C++ language projection (e.g. what is currently cppwinrt.exe).

**/tool/meta** implements the tool that converts between winmd and idl formats.

**/tool/java** implements the tool that generates the Java language projection.

**/tool/python** implements the tool that generates the Python language projection.

**/tool/dump** is a short-term tool for bootstrapping the meta_reader implementation.

### Scripts

The **/script** folder contains the scripts and tools used to build and bootstrap the various projects and language projections.

**bionicBuild.cmd** builds xlang for Ubuntu 18.04 via Windows Subsystem for Linux. Note, Ubuntu 18.04 is code named "Bionic Beaver" - hence the use of "bionic" in the script name.

**windowsBuild.cmd** builds xlang for Windows. Note, this must be run from a VS 2017 command prompt and you must have cmake and ninja build installed. 

**wslConfigBionic.sh** bash script to configure Ubuntu 18.04 on WSL with the tools needed for xlang

**bionicRunDump.cmd / windowsRunDump.cmd** runs the dump application from the _build folder for Ubuntu 18.04 on WSL and Windows respectively.

## Building

### Prerequisites

Xlang currently requires the following to be installed
* [Visual Studio 15.8.6](https://visualstudio.microsoft.com/downloads/) (or later)
* [Windows SDK 17663](http://go.microsoft.com/fwlink/?LinkID=2023014) (or later)
* [CMake 3.5](http://cmake.org/) (or later) 
* [Ninja build](https://ninja-build.org/)


### Known Issues

The following build failures indicate using outdated tools:

> xlang\platform\string_base.h(74): error C2327: 'xlang::impl::string_storage_base::alternate_form': is not a type name, static, or enumerator

    Solution: upgrade to Visual Studio 15.8.6 or later

> C:\Program Files (x86)\Windows Kits\10\include\10.0.17134.0\cppwinrt\winrt\base.h(2185): error C3861: 'from_abi': identifier not found

    Solution: upgrade to Windows SDK 17663 or later


### CMake

xlang uses CMake instead of vcxproj in order to support cross platform builds. 

### Visual Studio 2017

Visual Studio 2017 has built in CMake support. For best results, use the latest update.

1. Select File -> Open -> CMake... from the main menu, navigate to the root of this repo and select the CMakeLists.txt file.
2. Wait a bit for VS to parse the CMake files. 
    * The build files are generated in a folder under %USERPROFILE%\CMakeBuilds. You can easily open this folder in Explorer from VS by right clicking on any CMakeLists.txt file and selecting Cache -> Open Cache Folder. Note, CMake has a seperate cache folder for each build flavor (x86-Debug, x64-Release, etc) to store build files. 
3. To build, select CMake -> Build All from the main menu. As of this writing, there is only a single target (tool\dump\dump.exe) so there's little point to building only a subset of the repo. 
4. To enable F5 debugging, you can right-click on main.cpp in solution explorer and choose Set as Startup Item. You can also select dump.exe from the Select Startup Item dropdown on the standard toolbar in VS (i.e. the one with the big green arrow). 
    * You can also debug a specific app in the repo by right clicking a source file in Solution Explorer and selecting Debug from the context menu. For example, right clicking on tool/dump/main.cpp and selecting Debug will launch dump.exe under the debugger.

### Command Line

Using CMake is a two step process - you must generate the build files and then build. The build scripts described above execute both steps every time they are run, though re-running cmake is redundant. 

CMake must generate build files into a folder other than where the source files are located. As described above, VS generates the build files into a folder under %USERPROFILE%\CMakeBuilds. The command line scripts generate the build files into the _build subdirectory. This repo has configured git to ignore the _build subfolder.

