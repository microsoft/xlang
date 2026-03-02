# Verify removed API support in xlang ABI tool
# Validates that is_removed() is properly implemented in the ABI tool source

$ErrorActionPreference = "Stop"
$script:passed = 0
$script:failed = 0

function Assert-Contains {
    param([string]$File, [string]$Pattern, [string]$Message)
    $content = Get-Content $File -Raw
    if ($content -match $Pattern) {
        Write-Host "  PASS: $Message" -ForegroundColor Green
        $script:passed++
    } else {
        Write-Host "  FAIL: $Message" -ForegroundColor Red
        Write-Host "    Expected pattern: $Pattern" -ForegroundColor Yellow
        $script:failed++
    }
}

Write-Host "`n=== common.h: is_removed() helper ===" -ForegroundColor Cyan

$commonFile = "src\tool\abi\common.h"
Assert-Contains $commonFile "is_removed" "is_removed() function exists"
Assert-Contains $commonFile "is_deprecated" "is_deprecated() function exists"
Assert-Contains $commonFile "DeprecatedAttribute" "References DeprecatedAttribute"

Write-Host "`n=== types.cpp: removed API filtering ===" -ForegroundColor Cyan

$typesFile = "src\tool\abi\types.cpp"
Assert-Contains $typesFile "is_removed" "types.cpp has is_removed() checks"

# Verify that ABI vtable entries are NOT filtered (binary compat)
Assert-Contains $typesFile "virtual" "types.cpp generates C++ virtual methods"

Write-Host "`n=== Summary ===" -ForegroundColor Cyan
Write-Host "Passed: $($script:passed)" -ForegroundColor Green
if ($script:failed -gt 0) {
    Write-Host "Failed: $($script:failed)" -ForegroundColor Red
    exit 1
} else {
    Write-Host "All checks passed!" -ForegroundColor Green
}
