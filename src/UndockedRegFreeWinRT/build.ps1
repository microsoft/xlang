Param(
    [string]$Platform = "x64",
    [string]$Configuration = "debug",
    [switch]$Help = $false
)


$StartTime = Get-Date

if ($Help) {
    Write-Host @"
Copyright (c) Microsoft Corporation and Contributors.
Licensed under the MIT License.

Syntax:
      Build.cmd [options]

Description:
      Builds Undocked RegFree WinRT.

Options:

  -Platform <platform>
      Only build the selected platform(s)
      Example: -Platform x64
      Example: -Platform "x86,x64,arm64"

  -Configuration <configuration>
      Only build the selected configuration(s)
      Example: -Configuration release
      Example: -Configuration "debug,release"

  -Help
      Display this usage message.
"@
  Exit
}

$env:Build_RootDirectory = (Split-Path $MyInvocation.MyCommand.Path)
$env:Build_Platform = $Platform.ToLower()
$env:Build_Configuration = $Configuration.ToLower()

$msbuildPath = &"${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -latest -prerelease -products * -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe

$ErrorActionPreference = "Stop"

Try {
  foreach ($platform in $env:Build_Platform.Split(",")) {
    foreach ($configuration in $env:Build_Configuration.Split(",")) {
      $msbuildArgs = @(
          ("UndockedRegFreeWinRT.sln"),
          ("/p:platform="+$platform),
          ("/p:configuration="+$configuration),
          ("/restore"),
          ("/binaryLogger:URFW.$platform.$configuration.binlog")
      )

      & $msbuildPath $msbuildArgs
    }
  }
} Catch {
  $formatString = "`n{0}`n`n{1}`n`n"
  $fields = $_, $_.ScriptStackTrace
  Write-Host ($formatString -f $fields) -ForegroundColor RED
  Exit 1
}

$TotalTime = (Get-Date)-$StartTime
$TotalMinutes = [math]::Floor($TotalTime.TotalMinutes)
$TotalSeconds = [math]::Ceiling($TotalTime.TotalSeconds)

Write-Host @"

Total Running Time:
$TotalMinutes minutes and $TotalSeconds seconds
"@ -ForegroundColor CYAN