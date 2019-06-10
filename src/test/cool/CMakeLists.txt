
project(test_cool LANGUAGES CSharp)

# CMAKE doesn't have a good way to deal with tools which may
# generate multiple files not known a priori. See
# https://stackoverflow.com/questions/44076307/cmake-globbing-generated-files
# We could try to extract this list from "ildasm /text foo.winmd".
# For now, let's hardcode.
set(generated_files
    ${CMAKE_CURRENT_BINARY_DIR}/base-interop.cs
    ${CMAKE_CURRENT_BINARY_DIR}/TestCool.BasicEnum.cs
    ${CMAKE_CURRENT_BINARY_DIR}/TestCool.BasicStruct.cs
    ${CMAKE_CURRENT_BINARY_DIR}/TestCool.Class.cs
    ${CMAKE_CURRENT_BINARY_DIR}/TestCool.IClass.cs
    ${CMAKE_CURRENT_BINARY_DIR}/TestCool.IInterface.cs)

add_executable(test_cool "")
target_sources(test_cool
    PRIVATE main.cs
    testcool.idl
    ${generated_files}
)
set(CMAKE_CSharp_FLAGS "/langversion:7.1") # otherwise cmake defaults to 3
set_target_properties(test_cool PROPERTIES VS_DOTNET_TARGET_FRAMEWORK_VERSION "4.5.1")
set_target_properties(test_cool PROPERTIES VS_GLOBAL_LangVersion "7.1")
set_target_properties(test_cool PROPERTIES VS_GLOBAL_AllowUnsafeBlocks "true")


# Inputs and outputs. MIDL cannot handle / in paths.
file(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR}\\testcool.idl   TestCool_idl)
file(TO_NATIVE_PATH ${CMAKE_CURRENT_BINARY_DIR}\\testcool.winmd TestCool_winmd)

if( NOT EXISTS "$ENV{VS140COMNTOOLS}")
    message(FATAL_ERROR "VS140COMNTOOLS environment variable is not defined")
endif()

# Find WindowsKitsRoot10 (from registry)
get_filename_component(WindowsKitsRoot10 "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots;KitsRoot10]" ABSOLUTE)
if(NOT EXISTS "${WindowsKitsRoot10}")
    message(FATAL_ERROR "Could not find the Windows SDK Root")
endif()

# Find WindowsSDKContractDir (from CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION)
file(TO_NATIVE_PATH
    ${WindowsKitsRoot10}/References/${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}/Windows.Foundation.FoundationContract/3.0.0.0
    WindowsSDKContractDir)
if(NOT EXISTS "${WindowsSDKContractDir}")
    message(FATAL_ERROR "Contract not found at ${WindowsSDKContractDir}")
endif()

# Use midl from the associated windows SDK, not the random version in the VS path
set(MIDL_EXE ${WindowsKitsRoot10}/bin/${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}/x86/midl.exe)

# Command to create the winmd from idl
add_custom_command(
    OUTPUT ${TestCool_winmd}
    COMMENT Running MIDL on ${TestCool_idl}
    COMMAND "$ENV{VS140COMNTOOLS}\\vsvars32.bat"
    COMMAND ${MIDL_EXE}
        /input ${TestCool_idl}
        /out $<SHELL_PATH:${CMAKE_CURRENT_BINARY_DIR}>
        /w4 /nomidl /winrt
        /reference ${WindowsSDKContractDir}\\Windows.Foundation.FoundationContract.winmd
        /metadata_dir ${WindowsSDKContractDir}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${TestCool_idl}
    VERBATIM)

# Command to create the binding from winmd
add_custom_command(
    OUTPUT ${generated_files}
    COMMENT Running coolrt on on ${TestCool_winmd}
    COMMAND coolrt -input local -input ${TestCool_winmd} -out ${CMAKE_CURRENT_BINARY_DIR} -include TestCool    
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${TestCool_winmd}
    VERBATIM)
