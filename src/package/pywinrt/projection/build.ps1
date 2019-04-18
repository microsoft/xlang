$buildType = "Release"
# TODO: specify abi version number via param
$pythonTag = "cp37"

if ($env:VSCMD_ARG_TGT_ARCH -eq "x64") {
    $platformTag = "win_amd64"
}
elseif ($env:VSCMD_ARG_TGT_ARCH -eq "x86") {
    $platformTag = "win_x86"
}
else {
    Write-Error "VSCMD_ARG_TGT_ARCH not set to an expected value (x86 or x64)"
    exit 
}

$repoRootPath = (get-item $PSScriptRoot).parent.Parent.parent.Parent.FullName.Replace('\', '/')
$sourcePath = $PSScriptRoot.Replace('\', '/')
$buildPath = "$repoRootPath/_build/py-projection/$env:VSCMD_ARG_TGT_ARCH-$buildType"
$installPath = "$repoRootPath/_build/py-projection"

cmake -S $sourcePath "-B$buildPath" -GNinja "-DCMAKE_INSTALL_PREFIX=$installPath" "-DPYTHON_TAG=$pythonTag" "-DPLATFORM_TAG=$platformTag" "-DCMAKE_BUILD_TYPE=$buildType" -DCMAKE_C_COMPILER=cl -DCMAKE_CXX_COMPILER=cl -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON
cmake --build $buildPath -- install -v 

$binaryPath = resolve-path $installPath/*.pyd
if ($null -ne $binaryPath) {
    copy-item $binaryPath "$sourcePath/pywinrt/winrt"
}