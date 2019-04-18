param ([switch]$clean)

$windows_sdk = '10.0.17763.0'
$repoRootPath = (get-item $PSScriptRoot).parent.Parent.parent.Parent.FullName
$projectionPath = "$PSScriptRoot"

if ($clean)
{
    write-host "removing generated projection files"
    remove-item $projectionPath -rec -for -ErrorAction SilentlyContinue
    exit
}

$pywinrt_exe = Get-ChildItem $repoRootPath\_build\Windows\*\*\tool\python\pywinrt.exe | 
    Sort-Object -Descending | Select-Object -first 1

if (-not $pywinrt_exe) {
    Write-Error "pywinrt not avaliable"
    exit
}

nuget install Microsoft.Windows.CppWinRT -ExcludeVersion -OutputDirectory $projectionPath

# TODO: pass python version as param
$pythonVersion = "3.7.3"

if ($env:VSCMD_ARG_TGT_ARCH -eq "x64")
{
    nuget install python -ExcludeVersion -OutputDirectory $projectionPath -version $pythonVersion
}
elseif ($env:VSCMD_ARG_TGT_ARCH -eq "x86")
{
    nuget install pythonx86 -ExcludeVersion -OutputDirectory $projectionPath -version $pythonVersion
}
else 
{
    Write-Error "VSCMD_ARG_TGT_ARCH not set to an expected value (x86 or x64)"
    exit 
}

$cppwinrt_exe = "$projectionPath/Microsoft.Windows.CppWinRT\bin\cppwinrt.exe"

& $cppwinrt_exe -input $windows_sdk -output $projectionPath/cppwinrt -verbose

$pywinrt_path = "$projectionPath/pywinrt"

remove-item $pywinrt_path -Recurse -Force -ErrorAction SilentlyContinue

$pyinclude = "Windows.Data.Json", "Windows.Devices.Geolocation", "Windows.Foundation", "Windows.Graphics.DirectX"
#$pyinclude = @("Windows.")
#$pyexclude = "Windows.UI.Comp", "Windows.UI.Xaml"
$pyexclude = "Windows.UI.Composition", "Windows.UI.Xaml"
$pyin = $pyinclude | ForEach-Object{ "-include", "$_"}
$pyout = $pyexclude | ForEach-Object{ "-exclude", "$_"}

$pyparams = ("-input", $windows_sdk, "-output", $pywinrt_path, "-verbose") + $pyin + $pyout

& $pywinrt_exe $pyparams