$windows_sdk = '10.0.17763.0'
$repoRootPath = (get-item $PSScriptRoot).parent.Parent.parent.Parent.FullName

$pywinrt_exe = Get-ChildItem $repoRootPath\_build\Windows\*\*\tool\python\pywinrt.exe | 
    Sort-Object -Descending | Select-Object -first 1

if (-not $pywinrt_exe) {
    Write-Error "pywinrt not avaliable"
    exit
}

nuget install Microsoft.Windows.CppWinRT -ExcludeVersion -OutputDirectory $repoRootPath/_build/_tools
nuget install python -ExcludeVersion -OutputDirectory $PSScriptRoot
nuget install pythonx86 -ExcludeVersion -OutputDirectory $PSScriptRoot

$cppwinrt_exe = "$repoRootPath/_build/_tools/Microsoft.Windows.CppWinRT\bin\cppwinrt.exe"

& $cppwinrt_exe -in $windows_sdk -out $PSScriptRoot/cppwinrt -verbose

$pywinrt_path = "$PSScriptRoot/pywinrt"

remove-item $pywinrt_path -Recurse -Force

#$pyinclude = @("Windows.")
$pyinclude = "Windows.Data.Json", "Windows.Devices.Geolocation", "Windows.Foundation", "Windows.Graphics.DirectX"
#$pyexclude = "Windows.UI.Comp", "Windows.UI.Xaml"
$pyexclude = @() #"Windows.UI.Comp", "Windows.UI.Xaml"
$pyin = $pyinclude | ForEach-Object{ "-include", "$_"}
$pyout = $pyexclude | ForEach-Object{ "-exclude", "$_"}

$pyparams = ("-in", $windows_sdk, "-out", $pywinrt_path, "-verbose") + $pyin + $pyout

& $pywinrt_exe $pyparams
