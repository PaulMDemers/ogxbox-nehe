param(
    [string[]]$Lessons = @("1","2","3","4","5","6","7","8","9","10","11","12"),
    [ValidateSet("nxgl","pb","all")]
    [string]$Set = "all",
    [double]$DelaySeconds = 10.0,
    [ValidateRange(1,10)]
    [int]$LaunchAttempts = 3,
    [string]$CaptureSetName = "xemu_verified",
    [switch]$SkipBuild,
    [switch]$SkipCapture,
    [switch]$DebugRejectedCaptures,
    [switch]$AllowWarnings
)

$ErrorActionPreference = "Stop"

$repo = Resolve-Path (Join-Path $PSScriptRoot "..")
$workspace = Split-Path -Parent $repo

function Convert-ToMsysPath {
    param([string]$Path)

    $normalized = $Path -replace "\\", "/"
    if ($normalized -match "^([A-Za-z]):/(.*)$") {
        return "/$($matches[1].ToLowerInvariant())/$($matches[2])"
    }
    return $normalized
}

function Find-Bash {
    $candidates = @(
        "C:\devkitPro\msys2\usr\bin\bash.exe",
        "C:\msys64\usr\bin\bash.exe"
    )
    foreach ($candidate in $candidates) {
        if (Test-Path $candidate) {
            return $candidate
        }
    }
    throw "MSYS2 bash not found. Checked: $($candidates -join ', ')"
}

if (-not $SkipBuild) {
    $bash = Find-Bash
    $repoMsys = Convert-ToMsysPath $repo.Path
    $nxdk = Resolve-Path (Join-Path $workspace ".nxdk")
    $nxdkMsys = Convert-ToMsysPath $nxdk.Path
    $cmd = "cd '$repoMsys' && export MSYSTEM=MINGW64 NXDK_DIR='$nxdkMsys' PATH='$nxdkMsys/bin:$nxdkMsys/tools/cg/win:/mingw64/bin:/usr/bin:'`$PATH && make release"
    & $bash -lc $cmd
    if ($LASTEXITCODE -ne 0) {
        throw "NeHe release build failed."
    }
}

if (-not $SkipCapture) {
    $captureArgs = @{
        Set = $Set
        Lessons = $Lessons
        DelaySeconds = $DelaySeconds
        LaunchAttempts = $LaunchAttempts
        OutputSetName = $CaptureSetName
    }
    if ($DebugRejectedCaptures) {
        $captureArgs.DebugRejectedCaptures = $true
    }
    & (Join-Path $PSScriptRoot "capture_nehe_xemu.ps1") @captureArgs
}

$lessonInts = foreach ($lessonToken in $Lessons) {
    foreach ($lessonPart in ($lessonToken -split ",")) {
        if (-not [string]::IsNullOrWhiteSpace($lessonPart)) {
            [int]$lessonPart.Trim()
        }
    }
}

$focusLessons = @(5,6,7,12)
$compareScript = Join-Path $PSScriptRoot "generate_nehe_verified_compare.ps1"
if ($AllowWarnings) {
    & $compareScript `
        -Lessons $lessonInts `
        -FocusLessons $focusLessons `
        -CaptureSetName $CaptureSetName
} else {
    & $compareScript `
        -Lessons $lessonInts `
        -FocusLessons $focusLessons `
        -CaptureSetName $CaptureSetName `
        -Strict
}
