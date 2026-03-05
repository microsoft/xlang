# Verify removed API support in xlang ABI tool
# Validates:
# 1. is_removed() is properly implemented in source
# 2. Generated ABI headers preserve vtable slots for removed methods
# 3. Generated C macros skip removed methods
# 4. Removed types are excluded from output

param(
    [string]$AbiExePath = "src\tool\abi\x64\Release\abi.exe",
    [string]$WinMDPath = "G:\WinRT_DeprecationTesting\WinMD\DeprecationTest.winmd"
)

$ErrorActionPreference = "Stop"
$script:passed = 0
$script:failed = 0

function Assert-Contains {
    param([string]$Content, [string]$Pattern, [string]$Message, [switch]$Regex)
    if ($Regex) {
        if ($Content -match $Pattern) {
            Write-Host "  PASS: $Message" -ForegroundColor Green
            $script:passed++
        } else {
            Write-Host "  FAIL: $Message" -ForegroundColor Red
            Write-Host "    Expected regex: $Pattern" -ForegroundColor Yellow
            $script:failed++
        }
    } else {
        if ($Content -match [regex]::Escape($Pattern)) {
            Write-Host "  PASS: $Message" -ForegroundColor Green
            $script:passed++
        } else {
            Write-Host "  FAIL: $Message" -ForegroundColor Red
            Write-Host "    Expected to find: $Pattern" -ForegroundColor Yellow
            $script:failed++
        }
    }
}

function Assert-NotContains {
    param([string]$Content, [string]$Pattern, [string]$Message)
    if ($Content -match [regex]::Escape($Pattern)) {
        Write-Host "  FAIL: $Message" -ForegroundColor Red
        Write-Host "    Expected NOT to find: $Pattern" -ForegroundColor Yellow
        $script:failed++
    } else {
        Write-Host "  PASS: $Message" -ForegroundColor Green
        $script:passed++
    }
}

# === Source code verification ===
Write-Host "`n=== common.h: is_removed() helper ===" -ForegroundColor Cyan

$commonFile = "src\tool\abi\common.h"
$commonContent = Get-Content $commonFile -Raw
Assert-Contains $commonContent "is_removed" "is_removed() function exists"
Assert-Contains $commonContent "is_deprecated" "is_deprecated() function exists"
Assert-Contains $commonContent "DeprecatedAttribute" "References DeprecatedAttribute"

Write-Host "`n=== types.cpp: removed API filtering ===" -ForegroundColor Cyan

$typesFile = "src\tool\abi\types.cpp"
$typesContent = Get-Content $typesFile -Raw
Assert-Contains $typesContent "is_removed" "types.cpp has is_removed() checks"
Assert-Contains $typesContent "virtual" "types.cpp generates C++ virtual methods"

# === WinMD-based generation verification ===
if (!(Test-Path $WinMDPath)) {
    Write-Host "SKIP: WinMD not found at $WinMDPath" -ForegroundColor Yellow
} elseif (!(Test-Path $AbiExePath)) {
    Write-Host "SKIP: abi.exe not found at $AbiExePath" -ForegroundColor Yellow
} else {
    $abiExe = (Resolve-Path $AbiExePath).Path
    Write-Host "`nUsing abi.exe: $abiExe"

    $outDir = Join-Path $env:TEMP "xlang_verify_$(Get-Random)"
    New-Item -ItemType Directory -Path $outDir -Force | Out-Null

    & $abiExe -input $WinMDPath -reference local -output $outDir -include DeprecationTest -enable-header-deprecation 2>&1 | Out-Null

    $headerFile = Join-Path $outDir "DeprecationTest.h"
    if (!(Test-Path $headerFile)) {
        Write-Host "ERROR: Generated header not found" -ForegroundColor Red
        exit 1
    }

    $content = Get-Content $headerFile -Raw

    Write-Host "`n=== ABI vtable: ITestComponent removed methods preserved ===" -ForegroundColor Cyan
    Assert-Contains $content "GetEpsilon" "GetEpsilon vtable slot preserved in ABI" -Regex
    Assert-Contains $content "GetZeta" "GetZeta vtable slot preserved in ABI" -Regex
    Assert-Contains $content "get_RemovedProp" "get_RemovedProp vtable slot preserved" -Regex
    Assert-Contains $content "get_WritableRemovedProp" "get_WritableRemovedProp vtable slot preserved" -Regex
    Assert-Contains $content "put_WritableRemovedProp" "put_WritableRemovedProp vtable slot preserved" -Regex
    Assert-Contains $content "add_RemovedEvent" "add_RemovedEvent vtable slot preserved" -Regex
    Assert-Contains $content "remove_RemovedEvent" "remove_RemovedEvent vtable slot preserved" -Regex
    Assert-Contains $content "StaticRemovedMethod" "StaticRemovedMethod vtable slot preserved" -Regex
    Assert-Contains $content "get_StaticRemovedProp" "get_StaticRemovedProp vtable slot preserved" -Regex

    Write-Host "`n=== C macros: removed methods skipped ===" -ForegroundColor Cyan
    # C macros use __x_DeprecationTest_CITestComponent_MethodName pattern
    Assert-Contains $content "__x_DeprecationTest_CITestComponent_GetAlpha" "C macro for GetAlpha present" -Regex
    Assert-Contains $content "__x_DeprecationTest_CITestComponent_GetBeta" "C macro for GetBeta present" -Regex
    Assert-NotContains $content "__x_DeprecationTest_CITestComponent_GetEpsilon" "C macro for GetEpsilon skipped (removed)"
    Assert-NotContains $content "__x_DeprecationTest_CITestComponent_GetZeta" "C macro for GetZeta skipped (removed)"

    Write-Host "`n=== C macros: removed properties skipped ===" -ForegroundColor Cyan
    Assert-Contains $content "__x_DeprecationTest_CITestComponent_get_NormalProp" "C macro for get_NormalProp present" -Regex
    Assert-NotContains $content "__x_DeprecationTest_CITestComponent_get_RemovedProp" "C macro for get_RemovedProp skipped"
    Assert-Contains $content "__x_DeprecationTest_CITestComponent_get_WritableProp" "C macro for get_WritableProp present" -Regex
    Assert-NotContains $content "__x_DeprecationTest_CITestComponent_get_WritableRemovedProp" "C macro for get_WritableRemovedProp skipped"

    Write-Host "`n=== C macros: removed events skipped ===" -ForegroundColor Cyan
    Assert-Contains $content "__x_DeprecationTest_CITestComponent_add_NormalEvent" "C macro for add_NormalEvent present" -Regex
    Assert-NotContains $content "__x_DeprecationTest_CITestComponent_add_RemovedEvent" "C macro for add_RemovedEvent skipped"

    Write-Host "`n=== C macros: removed static methods skipped ===" -ForegroundColor Cyan
    Assert-Contains $content "__x_DeprecationTest_CITestComponentStatics_StaticMethod" "C macro for StaticMethod present" -Regex
    Assert-NotContains $content "__x_DeprecationTest_CITestComponentStatics_StaticRemovedMethod" "C macro for StaticRemovedMethod skipped"

    Write-Host "`n=== C macros: removed static properties skipped ===" -ForegroundColor Cyan
    Assert-Contains $content "__x_DeprecationTest_CITestComponentStatics_get_StaticProp" "C macro for get_StaticProp present" -Regex
    Assert-NotContains $content "__x_DeprecationTest_CITestComponentStatics_get_StaticRemovedProp" "C macro for get_StaticRemovedProp skipped"

    Write-Host "`n=== Removed types: excluded from output ===" -ForegroundColor Cyan
    Assert-NotContains $content "RemovedEnum_GoneValueA" "RemovedEnum values excluded"
    Assert-NotContains $content "interface IRemovedInterface" "IRemovedInterface excluded (removed types are always gone)"

    Write-Host "`n=== abi_writer.cpp: type-level removed exclusion ===" -ForegroundColor Cyan
    # The abi_writer fix ensures removed types don't generate type definitions.
    # Interfaces for removed classes are preserved (ABI compat), but enum/struct definitions are not.
    Assert-NotContains $content "RemovedEnum_GoneValueB" "RemovedEnum values not in type definitions"
    Assert-Contains $content "NormalEnum_ValueA" "NormalEnum values still generated" -Regex

    Write-Host "`n=== abi_writer.cpp: source code guards ===" -ForegroundColor Cyan
    $abiWriter = Get-Content "src\tool\abi\abi_writer.cpp" -Raw
    Assert-Contains $abiWriter "is_removed(type.get().type())" "abi_writer.cpp has is_removed() guard checks"
    Assert-Contains $abiWriter "is_removed(enumType.get().type())" "abi_writer.cpp filters removed enums"
    Assert-Contains $abiWriter "is_removed(classType.get().type())" "abi_writer.cpp filters removed classes"

    Write-Host "`n=== Normal types: present ===" -ForegroundColor Cyan
    Assert-Contains $content "NormalEnum_ValueA" "NormalEnum values present" -Regex
    Assert-Contains $content "interface INormalInterface" "INormalInterface present" -Regex

    Write-Host "`n=== PartiallyRemovedEnum: field removal ===" -ForegroundColor Cyan
    Assert-Contains $content "MixedEnum_Current" "MixedEnum_Current present" -Regex
    Assert-Contains $content "MixedEnum_Legacy" "MixedEnum_Legacy present" -Regex
    Assert-NotContains $content "MixedEnum_Removed" "MixedEnum_Removed excluded"

    # Cleanup
    Remove-Item -Recurse -Force $outDir -ErrorAction SilentlyContinue
}

Write-Host "`n=== Summary ===" -ForegroundColor Cyan
Write-Host "Passed: $($script:passed)" -ForegroundColor Green
if ($script:failed -gt 0) {
    Write-Host "Failed: $($script:failed)" -ForegroundColor Red
    exit 1
} else {
    Write-Host "All checks passed!" -ForegroundColor Green
}
