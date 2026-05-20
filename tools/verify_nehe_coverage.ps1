param(
    [int]$ExpectedLessonCount = 48,
    [switch]$CheckReleaseArtifacts,
    [string]$ReleaseRoot = ""
)

$ErrorActionPreference = "Stop"

$repo = Resolve-Path (Join-Path $PSScriptRoot "..")
$makefilePath = Join-Path $repo "Makefile"
$lessonsHeaderPath = Join-Path $repo "common_nehe\nehe_lessons.h"

$lessonLabels = @(
    "window","first_polygons","color","rotation","3d_shapes","texture_mapping",
    "filters_lighting","blending","moving_bitmaps","3d_world","flag_effect",
    "display_lists","bitmap_fonts","outline_fonts","texture_mapped_outline_fonts",
    "fog","texture_fonts","quadrics","particle_engine","masking",
    "lines_timing_ortho","bump_mapping","sphere_mapping","tokens_scissor_tga",
    "morphing_loading_objects","stencil_reflections","shadows","bezier_patches",
    "blitter_raw_textures","collision_detection","model_loading","picking_sorting",
    "tga_variants","height_map_terrain","avi_texture_playback","radial_blur",
    "cel_shading","resource_textures","physics_simulation","rope_physics",
    "volumetric_fog","multiple_viewports","freetype_fonts","lens_flare",
    "vertex_buffers","antialiasing","vertex_shader","arcball_rotation"
)

function Get-MakefileList {
    param(
        [string]$Text,
        [string]$Name
    )

    $match = [regex]::Match($Text, "(?ms)^$([regex]::Escape($Name))\s*=\s*\\\r?\n(?<body>.*?)(?:\r?\n\r?\n)")
    if (-not $match.Success) {
        throw "Could not find $Name in $makefilePath"
    }

    $items = @()
    foreach ($line in ($match.Groups["body"].Value -split "\r?\n")) {
        $item = ($line -replace "\\", "").Trim()
        if (-not [string]::IsNullOrWhiteSpace($item)) {
            $items += $item
        }
    }
    return $items
}

function Get-ExpectedAppName {
    param(
        [ValidateSet("nxgl","pb")]
        [string]$SetName,
        [int]$Lesson
    )

    $label = $lessonLabels[$Lesson - 1]
    if ($SetName -eq "nxgl") {
        $appNumber = if ($Lesson -le 12) { 110 + $Lesson } else { 200 + $Lesson }
        return ("{0}_nehe_nxgl_{1:D2}_{2}" -f $appNumber, $Lesson, $label)
    }

    $pbAppNumber = if ($Lesson -le 12) { 122 + $Lesson } else { 300 + $Lesson }
    return ("{0}_nehe_pb_{1:D2}_{2}" -f $pbAppNumber, $Lesson, $label)
}

function Add-Failure {
    param([string]$Message)
    $script:failures += $Message
}

function Get-AppIsoName {
    param([string]$App)

    $appMakefilePath = Join-Path $repo (Join-Path $App "Makefile")
    if (-not (Test-Path $appMakefilePath)) {
        return "xbnehe_$App.iso"
    }

    $appMakefile = Get-Content -LiteralPath $appMakefilePath -Raw
    if ($appMakefile -match "(?m)^XBE_TITLE\s*=\s*(\S+)") {
        return "$($matches[1]).iso"
    }
    return "xbnehe_$App.iso"
}

if ($ExpectedLessonCount -lt 1 -or $ExpectedLessonCount -gt $lessonLabels.Count) {
    throw "ExpectedLessonCount must be between 1 and $($lessonLabels.Count)."
}

$failures = @()
$makefile = Get-Content -LiteralPath $makefilePath -Raw
$header = Get-Content -LiteralPath $lessonsHeaderPath -Raw

if ($header -notmatch "#define\s+NEHE_LESSON_COUNT\s+(\d+)") {
    throw "Could not read NEHE_LESSON_COUNT from $lessonsHeaderPath"
}
$actualLessonCount = [int]$matches[1]
if ($actualLessonCount -ne $ExpectedLessonCount) {
    Add-Failure "NEHE_LESSON_COUNT is $actualLessonCount, expected $ExpectedLessonCount."
}

$nxglApps = @(Get-MakefileList $makefile "NEHE_NXGL_APPS")
$pbApps = @(Get-MakefileList $makefile "NEHE_PB_APPS")

if ($nxglApps.Count -ne $ExpectedLessonCount) {
    Add-Failure "NEHE_NXGL_APPS has $($nxglApps.Count) entries, expected $ExpectedLessonCount."
}
if ($pbApps.Count -ne $ExpectedLessonCount) {
    Add-Failure "NEHE_PB_APPS has $($pbApps.Count) entries, expected $ExpectedLessonCount."
}

$expectedNxgl = @()
$expectedPb = @()
for ($lesson = 1; $lesson -le $ExpectedLessonCount; ++$lesson) {
    $expectedNxgl += Get-ExpectedAppName "nxgl" $lesson
    $expectedPb += Get-ExpectedAppName "pb" $lesson
}

foreach ($expected in $expectedNxgl) {
    if ($nxglApps -notcontains $expected) {
        Add-Failure "Missing NXGL Makefile app: $expected"
    }
}
foreach ($unexpected in $nxglApps) {
    if ($expectedNxgl -notcontains $unexpected) {
        Add-Failure "Unexpected NXGL Makefile app: $unexpected"
    }
}
foreach ($expected in $expectedPb) {
    if ($pbApps -notcontains $expected) {
        Add-Failure "Missing PBKit Makefile app: $expected"
    }
}
foreach ($unexpected in $pbApps) {
    if ($expectedPb -notcontains $unexpected) {
        Add-Failure "Unexpected PBKit Makefile app: $unexpected"
    }
}

foreach ($app in @($expectedNxgl + $expectedPb)) {
    $appDir = Join-Path $repo $app
    if (-not (Test-Path $appDir)) {
        Add-Failure "Missing lesson directory: $app"
        continue
    }
    foreach ($requiredFile in @("Makefile", "main.c")) {
        if (-not (Test-Path (Join-Path $appDir $requiredFile))) {
            Add-Failure "Missing $requiredFile in $app"
        }
    }
}

if ($CheckReleaseArtifacts) {
    if ([string]::IsNullOrWhiteSpace($ReleaseRoot)) {
        $ReleaseRoot = Join-Path $repo "dist\release\nehe"
    }

    $isoRoot = Join-Path $ReleaseRoot "isos"
    $xbeRoot = Join-Path $ReleaseRoot "xbes"
    foreach ($root in @($isoRoot, $xbeRoot)) {
        if (-not (Test-Path $root)) {
            Add-Failure "Missing release artifact directory: $root"
        }
    }

    foreach ($app in @($expectedNxgl + $expectedPb)) {
        $isoPath = Join-Path $isoRoot (Get-AppIsoName $app)
        $xbePath = Join-Path $xbeRoot (Join-Path $app "default.xbe")
        if (-not (Test-Path $isoPath)) {
            Add-Failure "Missing release ISO: $isoPath"
        }
        if (-not (Test-Path $xbePath)) {
            Add-Failure "Missing release XBE: $xbePath"
        }
    }
}

if ($failures.Count -gt 0) {
    throw "NeHe coverage verification failed:`n$($failures -join "`n")"
}

$checkedArtifacts = if ($CheckReleaseArtifacts) { " with release artifacts" } else { "" }
Write-Host "Verified $ExpectedLessonCount NeHe lessons across NXGL and PBKit$checkedArtifacts."
