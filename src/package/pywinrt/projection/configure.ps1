$windows_sdk = '10.0.17763.0'
$repoRootPath = (get-item $PSScriptRoot).parent.Parent.parent.Parent.FullName
$buildPath = "$repoRootPath/_build/Windows/packages"
$toolsPath = "$buildPath\tools"

$pywinrt_exe = Get-ChildItem $repoRootPath\_build\Windows\*\*\tool\python\pywinrt.exe | 
    Sort-Object -Descending | Select-Object -first 1

if (-not $pywinrt_exe) {
    Write-Error "pywinrt not avaliable"
    exit
}

mkdir $buildPath\tools -ErrorAction SilentlyContinue | Out-Null

$nuget_url = "https://dist.nuget.org/win-x86-commandline/latest/nuget.exe"
$nuget_exe = join-path $toolsPath nuget.exe

if (-not(test-path $nuget_exe)) {
    Invoke-WebRequest $nuget_url -OutFile $nuget_exe
}

& $nuget_exe install Microsoft.Windows.CppWinRT -ExcludeVersion -OutputDirectory $buildPath\tools 
& $nuget_exe install python -ExcludeVersion -OutputDirectory $buildPath\tools
& $nuget_exe install pythonx86 -ExcludeVersion -OutputDirectory $buildPath\tools 

$cppwinrt_exe = resolve-path $buildPath\tools\Microsoft.Windows.CppWinRT\bin\cppwinrt.exe
$CPPWINRT_PATH = join-path $buildPath cppwinrt
& $cppwinrt_exe ("-in", $windows_sdk, "-out", $CPPWINRT_PATH, "-verbose")

$pywinrt_path = join-path $buildPath pywinrt

$include = @("Windows.")
# $include = "Windows.Data.Json", "Windows.Devices.Geolocation", "Windows.Foundation", "Windows.Graphics.DirectX"
$include_param = $include | ForEach-Object{ "-include", "$_"}
$exclude = "Windows.UI.Comp", "Windows.UI.Xaml"
$exclude_param = $exclude | ForEach-Object{ "-exclude", "$_"}

$all_param = ("-in", $windows_sdk, "-out", ${pywinrt_path}, "-verbose") + $include_param + $exclude_param
& $pywinrt_exe $all_param

