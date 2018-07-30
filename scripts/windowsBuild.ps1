Param (
    [ValidateSet('Debug', 'Release', 'RelWithDebInfo', 'MinSizeRel')] $buildType = "Debug",
    [switch] $forceCMake,
    [switch] $clean
)

if (-not (test-path env:VSINSTALLDIR)) {
    throw "windows build script must be run from a VS CMD prompt"
}

$rootPath = (split-path $PSScriptRoot)
$buildPath = join-path $rootPath "_build/Windows/$env:VSCMD_ARG_TGT_ARCH/$buildType"

$vswherePath = join-path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
$vsInstallPath = & $vswherepath -latest -property installationPath

$cmakePath = join-path $vsInstallPath "Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
$ninjaPath = join-path $vsInstallPath "Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe"

if ($forceCMake -or (-not (test-path (join-path $buildPath CMakeCache.txt)))) 
{
    & $cmakePath "$rootPath" "-B$buildPath" -G Ninja -DCMAKE_BUILD_TYPE="$buildType" -DCMAKE_MAKE_PROGRAM="$ninjaPath"
}

$ninjaArgs = if ($clean) { "clean" } else { "-v" }
& $ninjaPath -C "$buildPath" $ninjaArgs