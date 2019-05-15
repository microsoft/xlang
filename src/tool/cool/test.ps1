param([switch]$clean, [switch]$build)

$repoRootPath = (get-item $PSScriptRoot).Parent.Parent.Parent.FullName
$coolrtexe = Get-ChildItem (Join-Path $repoRootPath _build) "coolrt.exe" -Recurse | ForEach-Object{$_.FullName} | Select-Object -First 1
$outputDir = join-path $env:TEMP "coolrt"

if ($clean) { Remove-Item $outputDir -Recurse -force -ErrorAction SilentlyContinue }

& $coolrtexe -input local -output $outputDir  -include Windows.Foundation -exclude Windows.Foundation.Metadata -exclude Windows.Foundation.Diagnostics -include Windows.Data.Json

function Test-Any() {
    begin {
        $any = $false
    }
    process {
        $any = $true
    }
    end {
        $any
    }
}

if ($build) {

$testProjContent = @'
<Project Sdk="Microsoft.NET.Sdk">
  <PropertyGroup>
    <TargetFramework>netstandard2.0</TargetFramework>
    <AllowUnsafeBlocks>true</AllowUnsafeBlocks>
  </PropertyGroup>
</Project>
'@

    Set-Content (join-path $outputDir "test.csproj") $testProjContent
    dotnet build $outputDir
}