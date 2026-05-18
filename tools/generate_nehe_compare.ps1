param(
    [int[]]$Lessons = @(1,2,3,4,5,6,7,8,9,10,11,12),
    [int[]]$FocusLessons = @(5,6,7,12),
    [int]$TileWidth = 320,
    [int]$TileHeight = 240,
    [int]$FocusTileWidth = 480,
    [int]$FocusTileHeight = 360
)

$ErrorActionPreference = "Stop"

Add-Type -AssemblyName System.Drawing

$repo = Resolve-Path (Join-Path $PSScriptRoot "..")
$captureRoot = Join-Path $repo "dist\nehe_reference\captures"
$outDir = Join-Path $captureRoot "compare"
New-Item -ItemType Directory -Force -Path $outDir | Out-Null

$fontTitle = New-Object System.Drawing.Font("Arial", 14, [System.Drawing.FontStyle]::Bold)
$fontSmall = New-Object System.Drawing.Font("Arial", 10, [System.Drawing.FontStyle]::Bold)
$white = [System.Drawing.Brushes]::White
$background = New-Object System.Drawing.SolidBrush([System.Drawing.Color]::FromArgb(26, 26, 26))

function Copy-Bitmap {
    param([string]$Path)

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

function New-LessonSheet {
    param(
        [int]$Lesson,
        [int]$TileWidth,
        [int]$TileHeight
    )

    $num = "{0:D2}" -f $Lesson
    $paths = @(
        (Join-Path $captureRoot "windows\nehe_ref_$num.png"),
        (Join-Path $captureRoot "xemu\nxgl\nehe_nxgl_$num.png"),
        (Join-Path $captureRoot "xemu\pb\nehe_pb_$num.png")
    )
    $labels = @("Windows reference", "NXGL xemu", "PB xemu")
    $pad = 10
    $titleHeight = 28
    $labelHeight = 22
    $sheet = New-Object System.Drawing.Bitmap(($TileWidth * 3 + $pad * 4), ($TileHeight + $titleHeight + $labelHeight + $pad * 3))
    $graphics = [System.Drawing.Graphics]::FromImage($sheet)

    try {
        $graphics.FillRectangle($background, 0, 0, $sheet.Width, $sheet.Height)
        $graphics.DrawString("NeHe Lesson $num", $fontTitle, $white, $pad, 4)
        for ($i = 0; $i -lt 3; $i++) {
            $x = $pad + $i * ($TileWidth + $pad)
            $graphics.DrawString($labels[$i], $fontSmall, $white, $x, $titleHeight)
            $image = Copy-Bitmap $paths[$i]
            try {
                Draw-ImageFit $graphics $image $x ($titleHeight + $labelHeight) $TileWidth $TileHeight
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
        for ($i = 0; $i -lt $Sheets.Count; $i++) {
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
    $sheets = New-Object "System.Collections.Generic.List[System.Drawing.Bitmap]"
    foreach ($lesson in $Lessons) {
        $sheet = New-LessonSheet $lesson $TileWidth $TileHeight
        $sheets.Add($sheet)
        $sheet.Save((Join-Path $outDir ("nehe_compare_{0:D2}.png" -f $lesson)), [System.Drawing.Imaging.ImageFormat]::Png)
    }
    Save-Grid $sheets 2 (Join-Path $outDir "nehe_compare_all.png")
    foreach ($sheet in $sheets) {
        $sheet.Dispose()
    }

    $focusSheets = New-Object "System.Collections.Generic.List[System.Drawing.Bitmap]"
    foreach ($lesson in $FocusLessons) {
        $focusSheets.Add((New-LessonSheet $lesson $FocusTileWidth $FocusTileHeight))
    }
    Save-Grid $focusSheets 2 (Join-Path $outDir "nehe_compare_05_07_12_focus.png")
    foreach ($sheet in $focusSheets) {
        $sheet.Dispose()
    }
} finally {
    $fontTitle.Dispose()
    $fontSmall.Dispose()
    $background.Dispose()
}

Write-Host "Updated NeHe comparison sheets in $outDir"
