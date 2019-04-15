
$buildType = "Release"
$repoRootPath = (get-item $PSScriptRoot).parent.Parent.parent.Parent.FullName
$sourcePath = $PSScriptRoot.Replace('\', '/')
$buildPath = "$repoRootPath/_build/_pywinrt_projection/$env:VSCMD_ARG_TGT_ARCH-$buildType".Replace('\', '/')

# del $buildPath -rec -for
cmake -S $sourcePath "-B$buildPath" -GNinja -DCMAKE_BUILD_TYPE=$buildType -DCMAKE_C_COMPILER=cl -DCMAKE_CXX_COMPILER=cl -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON
cmake --build $buildPath -- -v 