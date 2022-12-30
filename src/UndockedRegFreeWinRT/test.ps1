Param(
  [string]$Platform = "x64",
  [string]$Configuration = "debug",
  [switch]$Help = $false
)

$StartTime = Get-Date

$env:Build_RootDirectory = (Split-Path $MyInvocation.MyCommand.Path)
$env:Build_Platform = $Platform.ToLower()
$env:Build_Configuration = $Configuration.ToLower()

Push-Location $env:Build_RootDirectory

Try {
  foreach ($platform in $env:Build_Platform.Split(",")) {
    foreach ($configuration in $env:Build_Configuration.Split(",")) {
      Write-Host ("Configuration: "+$platform+"|"+$configuration) -ForegroundColor CYAN
      Set-Location (".\_build\"+$platform+"\"+$configuration)
      .\UndockedRegFreeWinRTManagedTest.exe
      .\EmbeddedManifestManagedTest.exe
      .\ManifestParserTest.exe -s
      .\UndockedRegFreeWinRTTest.exe -s
      .\EmbeddedManifestTest.exe -s
    }
  }
} Catch {
  $formatString = "`n{0}`n`n{1}`n`n"
  $fields = $_, $_.ScriptStackTrace
  Write-Host ($formatString -f $fields) -ForegroundColor RED
  Exit 1
}
 
Pop-Location

$TotalTime = (Get-Date)-$StartTime
$TotalMinutes = [math]::Floor($TotalTime.TotalMinutes)
$TotalSeconds = [math]::Ceiling($TotalTime.TotalSeconds)

Write-Host @"

Total Running Time:
$TotalMinutes minutes and $TotalSeconds seconds
"@ -ForegroundColor CYAN