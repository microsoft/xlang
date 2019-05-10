$repoRootPath = (get-item $PSScriptRoot).Parent.Parent.Parent.FullName
$coolrtexe = Get-ChildItem (Join-Path $repoRootPath _build) "coolrt.exe" -Recurse | ForEach-Object{$_.FullName} | Select-Object -First 1
$outputDir = join-path $env:TEMP "coolrt"

Remove-Item $outputDir -Recurse -force -ErrorAction SilentlyContinue
& $coolrtexe -input local -output $outputDir -include Windows.Data.Json #-include Windows.Foundation -exclude Windows.Foundation.Metadata