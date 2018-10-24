Param (
    [ValidateSet('Debug', 'Release', 'RelWithDebInfo', 'MinSizeRel')] $buildType = "Debug",
    [switch] $forceCMake,
    [switch] $verbose,
    [string] $target
)

$bionicBuildPath = join-path $PSScriptRoot "bionicBuild.sh"
$wslBionicBuildPath = ubuntu1804 run wslpath ($bionicBuildPath.replace('\', '/'))

$args = "--build-type $buildType"
if ($forceCMake) { $args += " --force-cmake" }
if ($verbose) { $args += " --verbose" }
if (-not [string]::IsNullOrEmpty($target)) { $args += " --target $target" }

Write-Output "bash $wslBionicBuildPath $args"
ubuntu1804 run "bash $wslBionicBuildPath $args"