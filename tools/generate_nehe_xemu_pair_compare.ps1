param(
    [string[]]$Lessons = @("13","14","15","16","17","18","19"),
    [string]$CaptureSetName = "xemu_verified",
    [int]$TileWidth = 480,
    [int]$TileHeight = 336,
    [double]$MinNonDarkRatio = 0.002,
    [switch]$Strict
)

$ErrorActionPreference = "Stop"

Add-Type -AssemblyName System.Drawing

$repo = Resolve-Path (Join-Path $PSScriptRoot "..")
$captureRoot = Join-Path $repo "dist\nehe_reference\captures"
$outDir = Join-Path $captureRoot "compare"
New-Item -ItemType Directory -Force -Path $outDir | Out-Null

$fontTitle = New-Object System.Drawing.Font("Consolas", 14, [System.Drawing.FontStyle]::Bold)
$fontSmall = New-Object System.Drawing.Font("Consolas", 10, [System.Drawing.FontStyle]::Bold)
$white = [System.Drawing.Brushes]::White
$backgroundBrush = New-Object System.Drawing.SolidBrush([System.Drawing.Color]::FromArgb(20, 20, 20))

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
            if ($lesson -lt 1 -or $lesson -gt 99) {
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

function Copy-Bitmap {
    param([string]$Path)

    if (-not (Test-Path $Path)) {
        throw "Missing capture image: $Path"
    }

    $image = [System.Drawing.Image]::FromFile($Path)
    try {
        return New-Object System.Drawing.Bitmap($image)
    } finally {
        $image.Dispose()
    }
}

function Draw-ImageFit {
    param(
        [System.Drawing.Graphics]$Graphics,
        [System.Drawing.Bitmap]$Image,
        [int]$X,
        [int]$Y,
        [int]$Width,
        [int]$Height
    )

    $scale = [Math]::Min($Width / $Image.Width, $Height / $Image.Height)
    $drawWidth = [int]($Image.Width * $scale)
    $drawHeight = [int]($Image.Height * $scale)
    $drawX = $X + [int](($Width - $drawWidth) / 2)
    $drawY = $Y + [int](($Height - $drawHeight) / 2)
    $Graphics.DrawImage($Image, $drawX, $drawY, $drawWidth, $drawHeight)
}

function Get-ImageStats {
    param([string]$Path)

    $bitmap = Copy-Bitmap $Path
    try {
        $samples = 0
        $brightness = 0.0
        $nonDark = 0
        $minX = $bitmap.Width
        $minY = $bitmap.Height
        $maxX = -1
        $maxY = -1

        for ($y = 0; $y -lt $bitmap.Height; $y += 4) {
            for ($x = 0; $x -lt $bitmap.Width; $x += 4) {
                $pixel = $bitmap.GetPixel($x, $y)
                $value = ($pixel.R + $pixel.G + $pixel.B) / 3.0
                $brightness += $value
                $samples++
                if ($value -gt 12.0) {
                    $nonDark++
                    if ($x -lt $minX) { $minX = $x }
                    if ($y -lt $minY) { $minY = $y }
                    if ($x -gt $maxX) { $maxX = $x }
                    if ($y -gt $maxY) { $maxY = $y }
                }
            }
        }

        if ($samples -eq 0) {
            $samples = 1
        }
        if ($maxX -lt 0) {
            $minX = 0
            $minY = 0
            $maxX = 0
            $maxY = 0
        }

        return [ordered]@{
            path = $Path
            width = $bitmap.Width
            height = $bitmap.Height
            mean_brightness = [Math]::Round($brightness / $samples, 2)
            non_dark_sample_ratio = [Math]::Round($nonDark / $samples, 4)
            content_bounds = [ordered]@{
                x = $minX
                y = $minY
                width = [Math]::Max(0, $maxX - $minX + 1)
                height = [Math]::Max(0, $maxY - $minY + 1)
            }
        }
    } finally {
        $bitmap.Dispose()
    }
}

function Get-ImageComparison {
    param(
        [string]$ExpectedPath,
        [string]$ActualPath
    )

    $expected = Copy-Bitmap $ExpectedPath
    $actual = Copy-Bitmap $ActualPath
    try {
        $width = [Math]::Min($expected.Width, $actual.Width)
        $height = [Math]::Min($expected.Height, $actual.Height)
        if ($width -le 0 -or $height -le 0) {
            throw "Cannot compare empty image dimensions: $ExpectedPath vs $ActualPath"
        }

        $samples = 0
        $absoluteError = 0.0
        $expectedLit = 0
        $actualLit = 0
        $overlapLit = 0
        $unionLit = 0

        for ($y = 0; $y -lt $height; $y += 4) {
            for ($x = 0; $x -lt $width; $x += 4) {
                $e = $expected.GetPixel($x, $y)
                $a = $actual.GetPixel($x, $y)
                $absoluteError += ([Math]::Abs($e.R - $a.R) + [Math]::Abs($e.G - $a.G) + [Math]::Abs($e.B - $a.B)) / 3.0
                $samples++

                $eLit = (($e.R + $e.G + $e.B) / 3.0) -gt 12.0
                $aLit = (($a.R + $a.G + $a.B) / 3.0) -gt 12.0
                if ($eLit) { $expectedLit++ }
                if ($aLit) { $actualLit++ }
                if ($eLit -and $aLit) { $overlapLit++ }
                if ($eLit -or $aLit) { $unionLit++ }
            }
        }

        if ($samples -eq 0) {
            $samples = 1
        }

        $litIoU = 1.0
        if ($unionLit -gt 0) {
            $litIoU = $overlapLit / $unionLit
        }

        return [ordered]@{
            mean_absolute_rgb_error = [Math]::Round($absoluteError / $samples, 2)
            nxgl_lit_samples = $expectedLit
            pb_lit_samples = $actualLit
            lit_sample_iou = [Math]::Round($litIoU, 4)
        }
    } finally {
        $expected.Dispose()
        $actual.Dispose()
    }
}

function Get-LessonPaths {
    param([int]$Lesson)

    $num = "{0:D2}" -f $Lesson
    return [ordered]@{
        nxgl = Join-Path $captureRoot "$CaptureSetName\nxgl\nehe_nxgl_$num.png"
        pb = Join-Path $captureRoot "$CaptureSetName\pb\nehe_pb_$num.png"
    }
}

function New-LessonSheet {
    param(
        [int]$Lesson,
        [int]$Width,
        [int]$Height
    )

    $paths = Get-LessonPaths $Lesson
    $labels = @("NXGL xemu", "PBKit xemu")
    $images = @($paths.nxgl, $paths.pb)
    $pad = 12
    $titleHeight = 28
    $labelHeight = 22
    $sheet = New-Object System.Drawing.Bitmap(($Width * 2 + $pad * 3), ($Height + $titleHeight + $labelHeight + $pad * 3))
    $graphics = [System.Drawing.Graphics]::FromImage($sheet)

    try {
        $graphics.FillRectangle($backgroundBrush, 0, 0, $sheet.Width, $sheet.Height)
        $graphics.DrawString(("NeHe Lesson {0:D2}" -f $Lesson), $fontTitle, $white, $pad, 4)
        for ($i = 0; $i -lt 2; ++$i) {
            $x = $pad + $i * ($Width + $pad)
            $graphics.DrawString($labels[$i], $fontSmall, $white, $x, $titleHeight)
            $image = Copy-Bitmap $images[$i]
            try {
                Draw-ImageFit $graphics $image $x ($titleHeight + $labelHeight) $Width $Height
            } finally {
                $image.Dispose()
            }
        }
    } finally {
        $graphics.Dispose()
    }

    return $sheet
}

function Save-Grid {
    param(
        [System.Collections.Generic.List[System.Drawing.Bitmap]]$Sheets,
        [int]$Columns,
        [string]$Path
    )

    if ($Sheets.Count -eq 0) {
        return
    }

    $cellWidth = [int]$Sheets[0].Width
    $cellHeight = [int]$Sheets[0].Height
    $rows = [int][Math]::Ceiling($Sheets.Count / $Columns)
    $grid = New-Object System.Drawing.Bitmap(($cellWidth * $Columns), ($cellHeight * $rows))
    $graphics = [System.Drawing.Graphics]::FromImage($grid)

    try {
        $graphics.Clear([System.Drawing.Color]::Black)
        for ($i = 0; $i -lt $Sheets.Count; ++$i) {
            $x = ($i % $Columns) * $cellWidth
            $y = [Math]::Floor($i / $Columns) * $cellHeight
            $graphics.DrawImage($Sheets[$i], $x, $y, $cellWidth, $cellHeight)
        }
        $grid.Save($Path, [System.Drawing.Imaging.ImageFormat]::Png)
    } finally {
        $graphics.Dispose()
        $grid.Dispose()
    }
}

try {
    $lessonList = Convert-ToLessonList $Lessons
    $manifest = [ordered]@{
        capture_set = $CaptureSetName
        generated_at = (Get-Date).ToString("o")
        thresholds = [ordered]@{
            min_non_dark_sample_ratio = $MinNonDarkRatio
        }
        warnings = @()
        lessons = @()
    }

    $sheets = New-Object "System.Collections.Generic.List[System.Drawing.Bitmap]"
    foreach ($lesson in $lessonList) {
        $paths = Get-LessonPaths $lesson
        $lessonStats = [ordered]@{
            lesson = $lesson
            nxgl = Get-ImageStats $paths.nxgl
            pb = Get-ImageStats $paths.pb
            pb_vs_nxgl = Get-ImageComparison $paths.nxgl $paths.pb
        }

        foreach ($name in @("nxgl", "pb")) {
            $stats = $lessonStats[$name]
            $nonDarkRatio = [double]$stats["non_dark_sample_ratio"]
            if ($nonDarkRatio -lt $MinNonDarkRatio) {
                $manifest.warnings += "lesson $lesson $name capture looks mostly blank: non_dark_sample_ratio=$nonDarkRatio"
            }
        }

        $manifest.lessons += $lessonStats

        $sheet = New-LessonSheet $lesson $TileWidth $TileHeight
        $sheets.Add($sheet)
        $sheet.Save((Join-Path $outDir ("nehe_xemu_pair_compare_{0:D2}.png" -f $lesson)), [System.Drawing.Imaging.ImageFormat]::Png)
    }

    Save-Grid $sheets 2 (Join-Path $outDir "nehe_xemu_pair_compare_all.png")
    foreach ($sheet in $sheets) {
        $sheet.Dispose()
    }

    $manifestPath = Join-Path $outDir "nehe_xemu_pair_compare_metrics.json"
    $manifest | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $manifestPath -Encoding ASCII

    if ($Strict -and $manifest.warnings.Count -gt 0) {
        throw "xemu pair comparison generated warnings:`n$($manifest.warnings -join "`n")"
    }
} finally {
    $fontTitle.Dispose()
    $fontSmall.Dispose()
    $backgroundBrush.Dispose()
}

Write-Host "Updated xemu pair NeHe comparison sheets in $outDir"
