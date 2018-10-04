Param (
    [ValidateSet('Debug', 'Release', 'RelWithDebInfo', 'MinSizeRel')] $buildType = "Debug",
    [switch] $forceCMake,
    [switch] $verbose,
    [string] $target
)

$rootPath = split-path (split-path $PSScriptRoot)
$buildPath = join-path $rootPath "_build/Ubuntu_18.04/$buildType"

$wslRootPath = ubuntu1804 run wslpath ($rootPath.replace('\', '/'))
$wslBuildPath = ubuntu1804 run wslpath ($buildPath.replace('\', '/'))

if ($forceCMake -or (-not (test-path (join-path $buildPath CMakeCache.txt)))) 
{
    ubuntu1804 run cmake "$wslRootPath" "-B$wslBuildPath" -GNinja -DCMAKE_BUILD_TYPE="$buildType" -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang
}

ubuntu1804 run ninja -C "$wslBuildPath" $(if ($verbose) { "-v" }) $target