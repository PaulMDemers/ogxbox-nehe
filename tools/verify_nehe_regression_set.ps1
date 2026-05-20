param(
    [string[]]$Lessons = @("5","6","7","8","12"),
    [int]$TimeMs = 2000,
    [ValidateSet("nxgl","pb","all")]
    [string]$Set = "all",
    [double]$DelaySeconds = 8.0,
    [ValidateRange(1,10)]
    [int]$LaunchAttempts = 3,
    [string]$CaptureSetName = "xemu_regression_2000",
    [string]$Label = "",
    [switch]$DebugRejectedCaptures,
    [switch]$SkipSweep,
    [switch]$XemuOnly,
    [switch]$ResumeExisting,
    [switch]$AllowWarnings
)

$ErrorActionPreference = "Stop"

$repo = Resolve-Path (Join-Path $PSScriptRoot "..")
$captureRoot = Join-Path $repo "dist\nehe_reference\captures"
$labels = @(
    "window","first_polygons","color","rotation","3d_shapes","texture_mapping",
    "filters_lighting","blending","moving_bitmaps","3d_world","flag_effect",
    "display_lists","bitmap_fonts","outline_fonts","texture_mapped_outline_fonts","fog","texture_fonts","quadrics","particle_engine",
    "masking","lines_timing_ortho","bump_mapping","sphere_mapping","tokens_scissor_tga",
    "morphing_loading_objects","stencil_reflections","shadows","bezier_patches","blitter_raw_textures",
    "collision_detection","model_loading","picking_sorting","tga_variants","height_map_terrain",
    "avi_texture_playback","radial_blur","cel_shading","resource_textures","physics_simulation","rope_physics",
    "volumetric_fog","multiple_viewports","freetype_fonts","lens_flare","vertex_buffers","antialiasing","vertex_shader","arcball_rotation"
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

function Get-FrameSetNames {
    param([string]$RequestedSet)

    switch ($RequestedSet) {
        "nxgl" { return @("nxgl") }
        "pb" { return @("pb") }
        default { return @("nxgl","pb") }
    }
}

$lessonList = Convert-ToLessonList $Lessons
if ([string]::IsNullOrWhiteSpace($Label)) {
    $lessonText = ($lessonList | ForEach-Object { "{0:D2}" -f $_ }) -join "_"
    $Label = "regression_${lessonText}_${TimeMs}ms"
}

if (-not $SkipSweep) {
    $sweepArgs = @{
        Set = $Set
        Lessons = @($lessonList -join ",")
        Times = @([string]$TimeMs)
        DelaySeconds = $DelaySeconds
        LaunchAttempts = $LaunchAttempts
        Label = $Label
    }
    if ($DebugRejectedCaptures) {
        $sweepArgs.DebugRejectedCaptures = $true
    }
    if ($ResumeExisting) {
        $sweepArgs.ResumeExisting = $true
    }
    & (Join-Path $PSScriptRoot "capture_nehe_frame_sweep.ps1") @sweepArgs
}

$timeSuffix = "{0:D5}ms" -f $TimeMs
$frameRoot = Join-Path $captureRoot (Join-Path "xemu_frames" $Label)
if (-not (Test-Path $frameRoot)) {
    throw "Missing frame sweep output: $frameRoot"
}

$captureSetRoot = Join-Path $captureRoot $CaptureSetName
foreach ($setName in (Get-FrameSetNames $Set)) {
    New-Item -ItemType Directory -Force -Path (Join-Path $captureSetRoot $setName) | Out-Null
    foreach ($lesson in $lessonList) {
        $num = "{0:D2}" -f $lesson
        $src = Join-Path $frameRoot (Join-Path "${setName}_$num" ("{0}_{1}_{2}.png" -f $setName, $num, $timeSuffix))
        if (-not (Test-Path $src)) {
            throw "Missing fixed-time capture: $src"
        }
        $dst = Join-Path $captureSetRoot (Join-Path $setName ("nehe_{0}_{1}.png" -f $setName, $num))
        Copy-Item -LiteralPath $src -Destination $dst -Force
    }
}

if ($XemuOnly) {
    $compareScript = Join-Path $PSScriptRoot "generate_nehe_xemu_pair_compare.ps1"
    $compareArgs = @{
        CaptureSetName = $CaptureSetName
        Lessons = @($lessonList -join ",")
    }
    if (-not $AllowWarnings) {
        $compareArgs.Strict = $true
    }
    & $compareScript @compareArgs
} else {
    $compareScript = Join-Path $PSScriptRoot "generate_nehe_verified_compare.ps1"
    $compareArgs = @{
        CaptureSetName = $CaptureSetName
        Lessons = $lessonList
        FocusLessons = $lessonList
    }
    if (-not $AllowWarnings) {
        $compareArgs.Strict = $true
    }
    & $compareScript @compareArgs
}

Write-Host "Verified NeHe regression set '$CaptureSetName' from fixed time ${TimeMs}ms."
