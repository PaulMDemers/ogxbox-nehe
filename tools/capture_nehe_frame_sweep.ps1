param(
    [ValidateSet("nxgl","pb","all")]
    [string]$Set = "all",
    [int[]]$Lessons = @(5,6,7,8,12),
    [string[]]$Times = @("0","1000","2000","3000","4000","5000","6000"),
    [double]$DelaySeconds = 8.0,
    [string]$Label = "frame_sweep",
    [string]$NxdkDir = "",
    [switch]$SkipBuild
)

$ErrorActionPreference = "Stop"

$repo = Resolve-Path (Join-Path $PSScriptRoot "..")
$workspace = Split-Path -Parent $repo
$outRoot = Join-Path $repo (Join-Path "dist\nehe_reference\captures\xemu_frames" $Label)
$workCaptureSet = "xemu_frame_work"
$captureScript = Join-Path $PSScriptRoot "capture_nehe_xemu.ps1"
$labels = @(
    "window","first_polygons","color","rotation","3d_shapes","texture_mapping",
    "filters_lighting","blending","moving_bitmaps","3d_world","flag_effect",
    "display_lists","bitmap_fonts","outline_fonts","texture_mapped_outline_fonts","fog","texture_fonts","quadrics","particle_engine"
)

function Convert-ToTimeList {
    param([string[]]$Values)

    $out = @()
    foreach ($value in $Values) {
        foreach ($part in ($value -split ",")) {
            if ([string]::IsNullOrWhiteSpace($part)) {
                continue
            }
            $time = 0
            if (-not [int]::TryParse($part.Trim(), [ref]$time)) {
                throw "Invalid fixed time value: $part"
            }
            if ($time -lt 0 -or $time -gt 600000) {
                throw "Fixed time value out of range 0..600000: $time"
            }
            $out += $time
        }
    }
    if ($out.Count -eq 0) {
        throw "Pass at least one fixed time in milliseconds."
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
        "C:\msys64\usr\bin\bash.exe",
        "C:\devkitPro\msys2\usr\bin\bash.exe"
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

    if ($Lesson -lt 1 -or $Lesson -gt $labels.Count) {
        throw "Unsupported lesson number: $Lesson"
    }

    $label = $labels[$Lesson - 1]
    if ($SetName -eq "nxgl") {
        $appNumber = if ($Lesson -le 12) { 110 + $Lesson } else { 200 + $Lesson }
        if ($Lesson -le 12) {
            return ("{0}_nxgl_{1:D2}_{2}" -f $appNumber, $Lesson, $label)
        }
        return ("{0}_nehe_nxgl_{1:D2}_{2}" -f $appNumber, $Lesson, $label)
    }
    $appNumber = if ($Lesson -le 12) { 122 + $Lesson } else { 300 + $Lesson }
    if ($Lesson -le 12) {
        return ("{0}_pb_{1:D2}_{2}" -f $appNumber, $Lesson, $label)
    }
    return ("{0}_nehe_pb_{1:D2}_{2}" -f $appNumber, $Lesson, $label)
}

function Invoke-MsysBuild {
    param(
        [string]$AppName,
        [int]$TimeMs
    )

    if ($SkipBuild) {
        return
    }

    $repoMsys = Convert-ToMsysPath $repo.Path
    $nxdkMsys = Convert-ToMsysPath (Resolve-NxdkDir $NxdkDir)
    $script = @(
        "set -e",
        "cd '$repoMsys'",
        "export MSYSTEM=MINGW64",
        "export NXDK_DIR='$nxdkMsys'",
        "export PATH='$nxdkMsys/bin:$nxdkMsys/tools/cg/win:/mingw64/bin:/usr/bin:'`$PATH",
        "make -C '$AppName' clean >/dev/null",
        "make -C '$AppName' NEHE_CAPTURE_FIXED_TIME_MS=$TimeMs >/dev/null",
        "./tools/collect_release_isos.sh >/dev/null"
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
        throw "Timed NeHe build failed for $AppName at ${TimeMs}ms"
    }
}

$sets = switch ($Set) {
    "nxgl" { @("nxgl") }
    "pb" { @("pb") }
    default { @("nxgl","pb") }
}
$timeList = Convert-ToTimeList $Times

New-Item -ItemType Directory -Force -Path $outRoot | Out-Null
$results = @()

foreach ($setName in $sets) {
    foreach ($lesson in $Lessons) {
        $appName = Get-AppName $setName $lesson
        $lessonDir = Join-Path $outRoot ("{0}_{1:D2}" -f $setName, $lesson)
        New-Item -ItemType Directory -Force -Path $lessonDir | Out-Null

        foreach ($time in $timeList) {
            Invoke-MsysBuild $appName $time
            & $captureScript `
                -Set $setName `
                -Lessons @($lesson) `
                -DelaySeconds $DelaySeconds `
                -OutputSetName $workCaptureSet | Out-Null

            $src = Join-Path $repo ("dist\nehe_reference\captures\{0}\{1}\nehe_{1}_{2:D2}.png" -f $workCaptureSet, $setName, $lesson)
            if (-not (Test-Path $src)) {
                throw "Expected capture was not written: $src"
            }

            $dst = Join-Path $lessonDir ("{0}_{1:D2}_{2:D5}ms.png" -f $setName, $lesson, $time)
            Copy-Item -LiteralPath $src -Destination $dst -Force
            Write-Host "Captured $dst"
            $results += [ordered]@{
                set = $setName
                lesson = $lesson
                fixed_time_ms = $time
                app = $appName
                path = $dst
            }
        }
    }
}

$manifest = [ordered]@{
    generated_at = (Get-Date).ToString("o")
    set = $Set
    lessons = $Lessons
    times = $timeList
    delay_seconds = $DelaySeconds
    label = $Label
    captures = $results
}
$manifest | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $outRoot "manifest.json") -Encoding ASCII
