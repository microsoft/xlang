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

    if (-not(Get-ChildItem $outputDir *.csproj | test-any)) {
        dotnet new classlib -o $outputDir
    }

    dotnet build $outputDir
}