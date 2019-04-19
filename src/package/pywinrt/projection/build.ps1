
$buildType = "Release"
$repoRootPath = (get-item $PSScriptRoot).parent.Parent.parent.Parent.FullName
$packageRootPath = "$repoRootPath/_build/Windows/packages".Replace('\', '/')
$sourcePath = $PSScriptRoot.Replace('\', '/')
$buildPath = "$packageRootPath\$env:VSCMD_ARG_TGT_ARCH-$buildType".Replace('\', '/')

cmake $sourcePath "-B$buildPath" -GNinja -DCMAKE_BUILD_TYPE=$buildType -DCMAKE_C_COMPILER=cl -DCMAKE_CXX_COMPILER=cl "-DPACKAGE_ROOT_PATH=$packageRootPath"
cmake $sourcePath "-B$buildPath"
ninja -C "$buildPath" -v