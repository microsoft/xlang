
$repoRootPath = (get-item $PSScriptRoot).parent.Parent.parent.Parent.FullName
$projectionPath = "$PSScriptRoot"
$sourcePath = $projectionPath.Replace('\', '/')
$pywinrt_path = "$sourcePath/pywinrt"

$buildType = "Release"
$buildPath = "$repoRootPath/_build/py-projection/$env:VSCMD_ARG_TGT_ARCH-$buildType".Replace('\', '/')
# TODO: specify abi version number via param
$pythonTag = "cp37"

if ($env:VSCMD_ARG_TGT_ARCH -eq "x64")
{
    $platformTag = "win_amd64"
}
elseif ($env:VSCMD_ARG_TGT_ARCH -eq "x86")
{
    $platformTag = "win_x86"
}
else 
{
    Write-Error "VSCMD_ARG_TGT_ARCH not set to an expected value (x86 or x64)"
    exit 
}

cmake -S $sourcePath "-B$buildPath" -GNinja "-DPYTHON_TAG=$pythonTag" "-DPLATFORM_TAG=$platformTag" -DCMAKE_BUILD_TYPE=$buildType -DCMAKE_C_COMPILER=cl -DCMAKE_CXX_COMPILER=cl -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON
cmake --build $buildPath -- -v 
