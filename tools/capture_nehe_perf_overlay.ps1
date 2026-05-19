param(
    [ValidateSet("nxgl","pb","all")]
    [string]$Set = "all",
    [string[]]$Lessons = @("7","19"),
    [double]$DelaySeconds = 14.0,
    [ValidateRange(1,10)]
    [int]$LaunchAttempts = 3,
    [string]$OutputSetName = "xemu_perf_overlay",
    [string]$NxdkDir = "",
    [switch]$SkipBuild,
    [switch]$DebugRejectedCaptures
)

$ErrorActionPreference = "Stop"

$repo = Resolve-Path (Join-Path $PSScriptRoot "..")
$workspace = Split-Path -Parent $repo
$captureScript = Join-Path $PSScriptRoot "capture_nehe_xemu.ps1"
$labels = @(
    "window","first_polygons","color","rotation","3d_shapes","texture_mapping",
    "filters_lighting","blending","moving_bitmaps","3d_world","flag_effect",
    "display_lists","bitmap_fonts","outline_fonts","texture_mapped_outline_fonts","fog","texture_fonts","quadrics","particle_engine"
)

function Convert-ToLessonList {
    param([string[]]$Values)

    $out = @()
    foreach ($value in $Values) {
        foreach ($part in ($value -split ",")) {
            if ([string]::IsNullOrWhiteSpace($part)) {
                continue
            }
            $lesson = 0
            if (-not [int]::TryParse($part.Trim(), [ref]$lesson)) {
                throw "Invalid lesson number: $part"
            }
            if ($lesson -lt 1 -or $lesson -gt $labels.Count) {
                throw "Unsupported lesson number: $lesson"
            }
            $out += $lesson
        }
    }
    if ($out.Count -eq 0) {
        throw "Pass at least one lesson number."
    }
    return $out
}

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

function Resolve-NxdkDir {
    param([string]$Override)

    if (-not [string]::IsNullOrWhiteSpace($Override)) {
        return (Resolve-Path $Override).Path
    }
    if (-not [string]::IsNullOrWhiteSpace($env:NXDK_DIR) -and (Test-Path $env:NXDK_DIR)) {
        return (Resolve-Path $env:NXDK_DIR).Path
    }

    $candidate = Join-Path $workspace ".nxdk"
    if (Test-Path $candidate) {
        return (Resolve-Path $candidate).Path
    }

    throw "NXDK_DIR not set and no sibling .nxdk checkout found at $candidate"
}

function Get-AppName {
    param(
        [string]$SetName,
        [int]$Lesson
    )

    $label = $labels[$Lesson - 1]
    if ($SetName -eq "nxgl") {
        $appNumber = if ($Lesson -le 12) { 110 + $Lesson } else { 200 + $Lesson }
        return ("{0}_nehe_nxgl_{1:D2}_{2}" -f $appNumber, $Lesson, $label)
    }
    $pbNumber = if ($Lesson -le 12) { 122 + $Lesson } else { 300 + $Lesson }
    return ("{0}_nehe_pb_{1:D2}_{2}" -f $pbNumber, $Lesson, $label)
}

function Remove-PerfSensitiveObjects {
    param(
        [string]$SetName,
        [string]$AppName
    )

    $appDir = Join-Path $repo $AppName
    foreach ($path in @(
        (Join-Path $appDir "main.obj"),
        (Join-Path $appDir "main.c.d")
    )) {
        if (Test-Path $path) {
            Remove-Item -LiteralPath $path -Force
        }
    }

    if ($SetName -eq "pb") {
        foreach ($path in @(
            (Join-Path $repo "common_nehe\nehe_native.obj"),
            (Join-Path $repo "common_nehe\nehe_native.c.d")
        )) {
            if (Test-Path $path) {
                Remove-Item -LiteralPath $path -Force
            }
        }
    } elseif ($SetName -eq "nxgl") {
        $nxglRoot = Join-Path $workspace "nxgl"
        foreach ($path in @(
            (Join-Path $nxglRoot "src\backend\nxgl_backend.obj"),
            (Join-Path $nxglRoot "src\backend\nxgl_backend.c.d")
        )) {
            if (Test-Path $path) {
                Remove-Item -LiteralPath $path -Force
            }
        }
    }
}

function Invoke-PerfBuild {
    param(
        [string]$SetName,
        [string]$AppName
    )

    if ($SkipBuild) {
        return
    }

    Remove-PerfSensitiveObjects -SetName $SetName -AppName $AppName

    $repoMsys = Convert-ToMsysPath $repo.Path
    $nxdkMsys = Convert-ToMsysPath (Resolve-NxdkDir $NxdkDir)
    $script = @(
        "set -e",
        "cd '$repoMsys'",
        "export MSYSTEM=MINGW64",
        "export NXDK_DIR='$nxdkMsys'",
        "export PATH='$nxdkMsys/bin:$nxdkMsys/tools/cg/win:/mingw64/bin:/usr/bin:'`$PATH",
        "make -C '$AppName' NEHE_PERF_OVERLAY=1 >/dev/null"
    ) -join "`n"

    $previousErrorActionPreference = $ErrorActionPreference
    $ErrorActionPreference = "Continue"
    try {
        $output = & (Find-Bash) -lc $script 2>&1
        $exitCode = $LASTEXITCODE
    } finally {
        $ErrorActionPreference = $previousErrorActionPreference
    }

    if ($exitCode -ne 0) {
        Write-Host ($output -join "`n")
        throw "Perf NeHe build failed for $AppName"
    }
}

$sets = switch ($Set) {
    "nxgl" { @("nxgl") }
    "pb" { @("pb") }
    default { @("nxgl","pb") }
}
$lessonList = Convert-ToLessonList $Lessons

foreach ($lesson in $lessonList) {
    foreach ($setName in $sets) {
        $appName = Get-AppName -SetName $setName -Lesson $lesson
        $appDir = Join-Path $repo $appName

        Invoke-PerfBuild -SetName $setName -AppName $appName

        $captureArgs = @{
            Set = $setName
            Lessons = @([string]$lesson)
            DelaySeconds = $DelaySeconds
            LaunchAttempts = $LaunchAttempts
            OutputSetName = $OutputSetName
            IsoRoot = $appDir
        }
        if ($DebugRejectedCaptures) {
            $captureArgs.DebugRejectedCaptures = $true
        }
        & $captureScript @captureArgs
    }
}

Write-Host "Perf overlay captures written to dist/nehe_reference/captures/$OutputSetName"
