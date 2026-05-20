[CmdletBinding(SupportsShouldProcess = $true)]
param(
    [Parameter(Mandatory = $true)]
    [int]$Lesson,
    [string]$Title = "",
    [string]$Slug = "",
    [string]$Detail = "",
    [int]$NxglAppNumber = 0,
    [int]$PbAppNumber = 0,
    [switch]$UpdateMakefile,
    [switch]$AllowMissingImplementation,
    [switch]$Force
)

$ErrorActionPreference = "Stop"

$repo = Resolve-Path (Join-Path $PSScriptRoot "..")
$makefilePath = Join-Path $repo "Makefile"
$lessonsHeaderPath = Join-Path $repo "common_nehe\nehe_lessons.h"
$classicLessonCount = 48

$knownLessons = @{
    13 = @{ title = "Bitmap Fonts"; slug = "bitmap_fonts"; detail = "Bitmap font text rendering" }
    14 = @{ title = "Outline Fonts"; slug = "outline_fonts"; detail = "3D outline font rendering" }
    15 = @{ title = "Texture Mapped Outline Fonts"; slug = "texture_mapped_outline_fonts"; detail = "Texture mapped 3D text" }
    16 = @{ title = "Cool Looking Fog"; slug = "fog"; detail = "Fog blending controls" }
    17 = @{ title = "2D Texture Font"; slug = "texture_fonts"; detail = "Texture atlas font rendering" }
    18 = @{ title = "Quadrics"; slug = "quadrics"; detail = "GLU-style quadric shapes" }
    19 = @{ title = "Particle Engine"; slug = "particle_engine"; detail = "Additive blended particles" }
    20 = @{ title = "Masking"; slug = "masking"; detail = "Two-pass texture masking" }
    21 = @{ title = "Lines Timing Ortho"; slug = "lines_timing_ortho"; detail = "Timed antialiased line patterns" }
    22 = @{ title = "Bump Mapping"; slug = "bump_mapping"; detail = "Embossed multitexture-style cube" }
    23 = @{ title = "Sphere Mapping"; slug = "sphere_mapping"; detail = "Environment mapped sphere" }
    24 = @{ title = "Tokens Scissor TGA"; slug = "tokens_scissor_tga"; detail = "Scissor panes and TGA-style texture" }
    25 = @{ title = "Morphing Loading Objects"; slug = "morphing_loading_objects"; detail = "Point-cloud morphing between objects" }
    26 = @{ title = "Stencil Reflections"; slug = "stencil_reflections"; detail = "Reflected cube over clipped floor" }
    27 = @{ title = "Shadows"; slug = "shadows"; detail = "Planar shadow projection" }
    28 = @{ title = "Bezier Patches"; slug = "bezier_patches"; detail = "Textured Bezier-style patch" }
    29 = @{ title = "Blitter Raw Textures"; slug = "blitter_raw_textures"; detail = "Procedural raw texture blits" }
    30 = @{ title = "Collision Detection"; slug = "collision_detection"; detail = "Moving sphere/cube collision volume" }
    31 = @{ title = "Model Loading"; slug = "model_loading"; detail = "Loaded-style low-poly model mesh" }
    32 = @{ title = "Picking Sorting"; slug = "picking_sorting"; detail = "Sorted transparent selection panes" }
    33 = @{ title = "TGA Variants"; slug = "tga_variants"; detail = "Texture loader format comparison" }
    34 = @{ title = "Height Map Terrain"; slug = "height_map_terrain"; detail = "Procedural landscape height field" }
    35 = @{ title = "AVI Texture Playback"; slug = "avi_texture_playback"; detail = "Procedural video texture stream" }
    36 = @{ title = "Radial Blur"; slug = "radial_blur"; detail = "Layered render-to-texture style blur" }
    37 = @{ title = "Cel Shading"; slug = "cel_shading"; detail = "Quantized lighting and outlines" }
    38 = @{ title = "Resource Textures"; slug = "resource_textures"; detail = "Packed resource texture atlas" }
    39 = @{ title = "Physics Simulation"; slug = "physics_simulation"; detail = "Spring-mass field simulation" }
    40 = @{ title = "Rope Physics"; slug = "rope_physics"; detail = "Segmented rope simulation" }
    41 = @{ title = "Volumetric Fog"; slug = "volumetric_fog"; detail = "Layered translucent fog volume" }
    42 = @{ title = "Multiple Viewports"; slug = "multiple_viewports"; detail = "Four isolated viewport scenes" }
    43 = @{ title = "FreeType Fonts"; slug = "freetype_fonts"; detail = "Smooth glyph extrusion stand-in" }
    44 = @{ title = "Lens Flare"; slug = "lens_flare"; detail = "Billboard flare and occluder pass" }
    45 = @{ title = "Vertex Buffers"; slug = "vertex_buffers"; detail = "Streaming vertex field batches" }
    46 = @{ title = "Antialiasing"; slug = "antialiasing"; detail = "Subpixel line and edge smoothing" }
    47 = @{ title = "Vertex Shader"; slug = "vertex_shader"; detail = "Animated shader-style deformation" }
    48 = @{ title = "ArcBall Rotation"; slug = "arcball_rotation"; detail = "Trackball-controlled object rotation" }
}

function Convert-ToSlug {
    param([string]$Text)

    $slugText = $Text.ToLowerInvariant() -replace "[^a-z0-9]+", "_"
    $slugText = $slugText.Trim("_")
    if ([string]::IsNullOrWhiteSpace($slugText)) {
        throw "Could not derive a slug. Pass -Slug explicitly."
    }
    return $slugText
}

function Get-LessonCount {
    $header = Get-Content -LiteralPath $lessonsHeaderPath -Raw
    if ($header -notmatch "#define\s+NEHE_LESSON_COUNT\s+(\d+)") {
        throw "Could not read NEHE_LESSON_COUNT from $lessonsHeaderPath"
    }
    return [int]$matches[1]
}

function Get-AppNumber {
    param(
        [string]$SetName,
        [int]$LessonNumber
    )

    if ($SetName -eq "nxgl") {
        if ($LessonNumber -le 12) {
            return 110 + $LessonNumber
        }
        return 200 + $LessonNumber
    }

    if ($LessonNumber -le 12) {
        return 122 + $LessonNumber
    }
    return 300 + $LessonNumber
}

function Write-AsciiFile {
    param(
        [string]$Path,
        [string]$Text
    )

    if ((Test-Path $Path) -and -not $Force) {
        throw "Refusing to overwrite existing file: $Path"
    }
    Set-Content -LiteralPath $Path -Value $Text -Encoding ASCII
}

function Add-AppToMakefileList {
    param(
        [string]$ListName,
        [string]$AppName
    )

    $makefile = Get-Content -LiteralPath $makefilePath -Raw
    if ($makefile -match "(?m)^\s*$([regex]::Escape($AppName))\s*(\\)?\s*$") {
        return
    }

    $pattern = "(?ms)($([regex]::Escape($ListName))\s*=\s*\\\r?\n)(.*?)(\r?\n\r?\n)"
    $match = [regex]::Match($makefile, $pattern)
    if (-not $match.Success) {
        throw "Could not find $ListName in $makefilePath"
    }

    $body = $match.Groups[2].Value
    $lines = $body -split "\r?\n" | Where-Object { -not [string]::IsNullOrWhiteSpace($_) }
    if ($lines.Count -gt 0) {
        $last = $lines[$lines.Count - 1]
        if ($last.TrimEnd().EndsWith("\")) {
            $lines[$lines.Count - 1] = $last.TrimEnd()
        } else {
            $lines[$lines.Count - 1] = "$($last.TrimEnd()) \"
        }
    }
    $lines += "`t$AppName"
    $newBody = ($lines -join "`r`n")
    $updated = $makefile.Substring(0, $match.Index) + $match.Groups[1].Value + $newBody + $match.Groups[3].Value + $makefile.Substring($match.Index + $match.Length)
    Set-Content -LiteralPath $makefilePath -Value $updated -Encoding ASCII
}

if ($Lesson -lt 1 -or $Lesson -gt 99) {
    throw "Lesson must be between 1 and 99."
}

if ([string]::IsNullOrWhiteSpace($Title) -and $knownLessons.ContainsKey($Lesson)) {
    $Title = $knownLessons[$Lesson].title
}
if ([string]::IsNullOrWhiteSpace($Slug) -and $knownLessons.ContainsKey($Lesson)) {
    $Slug = $knownLessons[$Lesson].slug
}
if ([string]::IsNullOrWhiteSpace($Detail) -and $knownLessons.ContainsKey($Lesson)) {
    $Detail = $knownLessons[$Lesson].detail
}
if ([string]::IsNullOrWhiteSpace($Title)) {
    if ($Lesson -gt $classicLessonCount) {
        throw "Pass -Title for supplemental demos after the classic NeHe lesson 48 ceiling."
    }
    throw "Pass -Title for lessons that are not in the built-in metadata map."
}
if ([string]::IsNullOrWhiteSpace($Slug)) {
    $Slug = Convert-ToSlug $Title
}
if ([string]::IsNullOrWhiteSpace($Detail)) {
    $Detail = $Title
}

$lessonCount = Get-LessonCount
if ($Lesson -gt $lessonCount -and -not $AllowMissingImplementation) {
    throw "NEHE_LESSON_COUNT is $lessonCount. The classic NeHe sequence ends at $classicLessonCount in this repository; implement common_nehe lesson $Lesson first or pass -AllowMissingImplementation for an explicitly supplemental wrapper."
}
if ($Lesson -gt $lessonCount -and $UpdateMakefile -and -not $AllowMissingImplementation) {
    throw "Refusing to add lesson $Lesson to the build before common_nehe implements it."
}

if ($NxglAppNumber -eq 0) {
    $NxglAppNumber = Get-AppNumber "nxgl" $Lesson
}
if ($PbAppNumber -eq 0) {
    $PbAppNumber = Get-AppNumber "pb" $Lesson
}

$lessonIndex = $Lesson - 1
$lessonPadded = "{0:D2}" -f $Lesson
$nxglApp = "{0}_nehe_nxgl_{1}_{2}" -f $NxglAppNumber, $lessonPadded, $Slug
$pbApp = "{0}_nehe_pb_{1}_{2}" -f $PbAppNumber, $lessonPadded, $Slug
$nxglDir = Join-Path $repo $nxglApp
$pbDir = Join-Path $repo $pbApp

$nxglMakefile = @"
XBE_TITLE = xbnehe_$nxglApp
GEN_XISO = `$`(XBE_TITLE).iso
NXDK_DIR ?= `$`(CURDIR)/../../.nxdk
NXGL_DIR ?= `$`(CURDIR)/../../nxgl

include `$`(NXGL_DIR)/nxgl.mk

SRCS = `$`(NXGL_SRCS) `$`(CURDIR)/main.c
SHADER_OBJS = `$`(NXGL_SHADER_OBJS)
CFLAGS += -I`$`(CURDIR)/../common_nehe `$`(NXGL_CFLAGS)
include `$`(CURDIR)/../common_nehe/nehe_capture.mk
include `$`(NXDK_DIR)/Makefile
"@

$nxglMain = @"
#define NEHE_STANDALONE_LESSON_INDEX $lessonIndex
#include "../110_nehe_nxgl_demo_disk/main.c"
"@

$pbMakefile = @"
XBE_TITLE = xbnehe_$pbApp
GEN_XISO = `$`(XBE_TITLE).iso
SRCS = `$`(CURDIR)/../common_nehe/nehe_native.c `$`(CURDIR)/../common_nehe/nehe_lessons.c `$`(CURDIR)/main.c
SHADER_OBJS = `$`(CURDIR)/../common3d/ps.inl `$`(CURDIR)/../common3d/vs.inl `$`(CURDIR)/../common_nehe/nehe_tex_ps.inl `$`(CURDIR)/../common_nehe/nehe_tex_replace_ps.inl `$`(CURDIR)/../common_nehe/nehe_tex_decal_ps.inl `$`(CURDIR)/../common_nehe/nehe_tex_blend_ps.inl `$`(CURDIR)/../common_nehe/nehe_tex_add_ps.inl `$`(CURDIR)/../common_nehe/nehe_tex_subtract_ps.inl `$`(CURDIR)/../common_nehe/nehe_tex_add_signed_ps.inl `$`(CURDIR)/../common_nehe/nehe_tex_interpolate_ps.inl `$`(CURDIR)/../common_nehe/nehe_tex_vs.inl `$`(CURDIR)/../common_nehe/nehe_tex2_vs.inl `$`(CURDIR)/../common_nehe/nehe_tex2_modulate_ps.inl `$`(CURDIR)/../common_nehe/nehe_cube_ps.inl `$`(CURDIR)/../common_nehe/nehe_tex3d_ps.inl
NXDK_DIR ?= `$`(CURDIR)/../../.nxdk
CFLAGS += -I`$`(CURDIR)/../common_nehe

include `$`(CURDIR)/../common_nehe/nehe_capture.mk
include `$`(NXDK_DIR)/Makefile
"@

$pbMain = @"
#include "nehe_lessons.h"
#include "nehe_native.h"
#include "nehe_capture_time.h"

#include <stdio.h>
#include <windows.h>

#define STANDALONE_LESSON_INDEX $lessonIndex

int main(void)
{
    DWORD start;

    if (n3_init() != 0) {
        Sleep(3000);
        return 1;
    }
    if (nehe_lessons_init() != 0) {
        Sleep(3000);
        return 1;
    }

    start = GetTickCount();
    while (1) {
        char title[96];
        float t = nehe_frame_seconds(start);

        snprintf(title, sizeof(title), "pb NeHe %02d standalone", STANDALONE_LESSON_INDEX + 1);
        n3_begin(nehe_lesson_clear_color(STANDALONE_LESSON_INDEX), nehe_lesson_blend_enabled(STANDALONE_LESSON_INDEX));
        nehe_lesson_render(STANDALONE_LESSON_INDEX, t);
        n3_finish(title, nehe_lesson_detail(STANDALONE_LESSON_INDEX));
    }

    nehe_lessons_shutdown();
    n3_shutdown();
    return 0;
}
"@

$actions = @(
    "NXGL app: $nxglApp",
    "PB app: $pbApp",
    "Lesson index: $lessonIndex",
    "Update Makefile: $($UpdateMakefile.IsPresent)"
)
Write-Host ($actions -join "`n")

if ($PSCmdlet.ShouldProcess("$nxglApp and $pbApp", "Create NeHe lesson scaffold")) {
    foreach ($dir in @($nxglDir, $pbDir)) {
        if ((Test-Path $dir) -and -not $Force) {
            throw "Refusing to use existing directory without -Force: $dir"
        }
        New-Item -ItemType Directory -Force -Path $dir | Out-Null
    }

    Write-AsciiFile (Join-Path $nxglDir "Makefile") $nxglMakefile
    Write-AsciiFile (Join-Path $nxglDir "main.c") $nxglMain
    Write-AsciiFile (Join-Path $pbDir "Makefile") $pbMakefile
    Write-AsciiFile (Join-Path $pbDir "main.c") $pbMain

    if ($UpdateMakefile) {
        Add-AppToMakefileList "NEHE_NXGL_APPS" $nxglApp
        Add-AppToMakefileList "NEHE_PB_APPS" $pbApp
    }
}
