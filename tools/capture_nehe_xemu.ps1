param(
    [ValidateSet("nxgl","pb","all")]
    [string]$Set = "all",
    [string[]]$Lessons = @("1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16","17","18","19","20","21","22","23","24","25","26","27","28","29","30","31","32","33","34","35","36","37","38","39","40","41","42","43","44","45","46","47","48"),
    [double]$DelaySeconds = 12.0,
    [string]$OutputSetName = "xemu",
    [string]$IsoRoot = "",
    [string]$EmuRoot = "",
    [ValidateRange(1,10)]
    [int]$LaunchAttempts = 3,
    [switch]$NoSnapshot,
    [switch]$UseSnapshot,
    [switch]$DebugRejectedCaptures
)

$ErrorActionPreference = "Stop"

$repo = Resolve-Path (Join-Path $PSScriptRoot "..")
if ([string]::IsNullOrWhiteSpace($IsoRoot)) {
    $isoRoot = Join-Path $repo "dist\release\nehe\isos"
} else {
    $isoRoot = (Resolve-Path $IsoRoot).Path
}
$captureRoot = Join-Path $repo (Join-Path "dist\nehe_reference\captures" $OutputSetName)

if ([string]::IsNullOrWhiteSpace($EmuRoot)) {
    $emuCandidates = @(
        (Join-Path $repo "Xbox-Emulator-Files"),
        (Join-Path (Split-Path -Parent $repo) "Xbox-Emulator-Files")
    )
    foreach ($candidate in $emuCandidates) {
        if (Test-Path (Join-Path $candidate "xemu\xemu.exe")) {
            $EmuRoot = $candidate
            break
        }
    }
    if ([string]::IsNullOrWhiteSpace($EmuRoot)) {
        $EmuRoot = $emuCandidates[0]
    }
}

$xemuRoot = Join-Path $EmuRoot "xemu"
$xemuExe = Join-Path $xemuRoot "xemu.exe"
$configPath = Join-Path $xemuRoot "xemu-local.toml"
$runXemu = Join-Path $PSScriptRoot "run_xemu.ps1"
$lessonLabels = @(
    "window","first_polygons","color","rotation","3d_shapes","texture_mapping",
    "filters_lighting","blending","moving_bitmaps","3d_world","flag_effect",
    "display_lists","bitmap_fonts","outline_fonts","texture_mapped_outline_fonts","fog","texture_fonts","quadrics","particle_engine",
    "masking","lines_timing_ortho","bump_mapping","sphere_mapping","tokens_scissor_tga",
    "morphing_loading_objects","stencil_reflections","shadows","bezier_patches","blitter_raw_textures",
    "collision_detection","model_loading","picking_sorting","tga_variants","height_map_terrain",
    "avi_texture_playback","radial_blur","cel_shading","resource_textures","physics_simulation","rope_physics",
    "volumetric_fog","multiple_viewports","freetype_fonts","lens_flare","vertex_buffers","antialiasing","vertex_shader","arcball_rotation"
)

Add-Type -AssemblyName System.Drawing
Add-Type @"
using System;
using System.Runtime.InteropServices;

public static class XemuCapture {
    [StructLayout(LayoutKind.Sequential)]
    public struct RECT {
        public int Left;
        public int Top;
        public int Right;
        public int Bottom;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct POINT {
        public int X;
        public int Y;
    }

    [DllImport("user32.dll")]
    public static extern bool GetClientRect(IntPtr hWnd, out RECT rect);

    [DllImport("user32.dll")]
    public static extern bool GetWindowRect(IntPtr hWnd, out RECT rect);

    [DllImport("user32.dll")]
    public static extern bool ClientToScreen(IntPtr hWnd, ref POINT point);

    [DllImport("user32.dll")]
    public static extern bool SetWindowPos(IntPtr hWnd, IntPtr hWndInsertAfter, int X, int Y, int cx, int cy, uint uFlags);

    [DllImport("user32.dll")]
    public static extern bool ShowWindow(IntPtr hWnd, int nCmdShow);

    [DllImport("user32.dll")]
    public static extern bool SetForegroundWindow(IntPtr hWnd);

    [DllImport("user32.dll")]
    public static extern bool SetProcessDPIAware();

    [DllImport("user32.dll")]
    public static extern bool PrintWindow(IntPtr hwnd, IntPtr hdcBlt, uint nFlags);

    public delegate bool EnumWindowsProc(IntPtr hWnd, IntPtr lParam);

    [DllImport("user32.dll")]
    public static extern bool EnumWindows(EnumWindowsProc lpEnumFunc, IntPtr lParam);

    [DllImport("user32.dll")]
    public static extern int GetWindowTextLength(IntPtr hWnd);

    [DllImport("user32.dll", CharSet = CharSet.Unicode)]
    public static extern int GetWindowText(IntPtr hWnd, System.Text.StringBuilder text, int count);

    [DllImport("user32.dll")]
    public static extern uint GetWindowThreadProcessId(IntPtr hWnd, out uint processId);

    [DllImport("user32.dll")]
    public static extern bool PostMessage(IntPtr hWnd, uint msg, IntPtr wParam, IntPtr lParam);

    public static readonly IntPtr HWND_TOPMOST = new IntPtr(-1);
    public const uint SWP_SHOWWINDOW = 0x0040;
    public const int SW_RESTORE = 9;
    public const uint PW_RENDERFULLCONTENT = 0x00000002;
    public const uint WM_CLOSE = 0x0010;
}
"@

[XemuCapture]::SetProcessDPIAware() | Out-Null

function Get-MainWindowHandle {
    param([System.Diagnostics.Process]$Process)

    for ($i = 0; $i -lt 150; $i++) {
        $Process.Refresh()
        if ($Process.HasExited) {
            throw "xemu exited before capture"
        }
        if ($Process.MainWindowHandle -ne [IntPtr]::Zero) {
            return $Process.MainWindowHandle
        }
        Start-Sleep -Milliseconds 100
    }
    throw "No xemu window handle for process $($Process.Id)"
}

function Set-XemuCaptureWindow {
    param([IntPtr]$Handle)

    [XemuCapture]::ShowWindow($Handle, [XemuCapture]::SW_RESTORE) | Out-Null
    [XemuCapture]::SetWindowPos($Handle, [XemuCapture]::HWND_TOPMOST, 20, 20, 800, 600, [XemuCapture]::SWP_SHOWWINDOW) | Out-Null
    [XemuCapture]::SetForegroundWindow($Handle) | Out-Null
}

function Close-XemuUpdateDialogs {
    param([System.Diagnostics.Process]$Process)

    $targetPid = [uint32]$Process.Id
    $callback = [XemuCapture+EnumWindowsProc]{
        param([IntPtr]$hWnd, [IntPtr]$lParam)

        $windowPid = [uint32]0
        [XemuCapture]::GetWindowThreadProcessId($hWnd, [ref]$windowPid) | Out-Null
        if ($windowPid -ne $targetPid) {
            return $true
        }

        $length = [XemuCapture]::GetWindowTextLength($hWnd)
        if ($length -le 0) {
            return $true
        }

        $builder = New-Object System.Text.StringBuilder ($length + 1)
        [XemuCapture]::GetWindowText($hWnd, $builder, $builder.Capacity) | Out-Null
        $title = $builder.ToString()
        if ($title -match '(?i)update|newer version|version of xemu') {
            [XemuCapture]::PostMessage($hWnd, [XemuCapture]::WM_CLOSE, [IntPtr]::Zero, [IntPtr]::Zero) | Out-Null
        }
        return $true
    }

    [XemuCapture]::EnumWindows($callback, [IntPtr]::Zero) | Out-Null
}

function Get-CaptureStats {
    param([System.Drawing.Bitmap]$Bitmap)

    $samples = 0
    $total = 0.0
    $nonDark = 0
    $cornerSamples = 0
    $cornerBrightNeutral = 0
    $upperRightSamples = 0
    $upperRightBrightNeutral = 0
    for ($y = 0; $y -lt $Bitmap.Height; $y += 16) {
        for ($x = 0; $x -lt $Bitmap.Width; $x += 16) {
            $pixel = $Bitmap.GetPixel($x, $y)
            $value = ($pixel.R + $pixel.G + $pixel.B) / 3.0
            $total += $value
            if ($value -gt 12.0) {
                $nonDark++
            }
            if (($x -ge ($Bitmap.Width * 0.8)) -and ($y -ge ($Bitmap.Height * 0.8))) {
                $cornerSamples++
                $maxChannel = [Math]::Max($pixel.R, [Math]::Max($pixel.G, $pixel.B))
                $minChannel = [Math]::Min($pixel.R, [Math]::Min($pixel.G, $pixel.B))
                if (($pixel.R -gt 180) -and ($pixel.G -gt 180) -and ($pixel.B -gt 180) -and (($maxChannel - $minChannel) -lt 35)) {
                    $cornerBrightNeutral++
                }
            }
            if (($x -ge ($Bitmap.Width * 0.55)) -and ($y -ge ($Bitmap.Height * 0.15)) -and ($y -le ($Bitmap.Height * 0.35))) {
                $upperRightSamples++
                $maxChannel = [Math]::Max($pixel.R, [Math]::Max($pixel.G, $pixel.B))
                $minChannel = [Math]::Min($pixel.R, [Math]::Min($pixel.G, $pixel.B))
                if (($pixel.R -gt 180) -and ($pixel.G -gt 180) -and ($pixel.B -gt 180) -and (($maxChannel - $minChannel) -lt 35)) {
                    $upperRightBrightNeutral++
                }
            }
            $samples++
        }
    }

    if ($samples -eq 0) {
        $samples = 1
    }
    if ($cornerSamples -eq 0) {
        $cornerSamples = 1
    }
    if ($upperRightSamples -eq 0) {
        $upperRightSamples = 1
    }

    return [ordered]@{
        mean_brightness = $total / $samples
        non_dark_ratio = $nonDark / $samples
        corner_bright_neutral_ratio = $cornerBrightNeutral / $cornerSamples
        upper_right_bright_neutral_ratio = $upperRightBrightNeutral / $upperRightSamples
    }
}

function Test-CaptureLooksLikeFramebuffer {
    param([System.Drawing.Bitmap]$Bitmap)

    if ($Bitmap.Width -lt 320 -or $Bitmap.Height -lt 240) {
        return $false
    }

    $stats = Get-CaptureStats -Bitmap $Bitmap
    return (($stats.mean_brightness -lt 220.0) -and ($stats.non_dark_ratio -gt 0.002) -and ($stats.corner_bright_neutral_ratio -lt 0.08) -and ($stats.upper_right_bright_neutral_ratio -lt 0.65))
}

function Get-RowMeanBrightness {
    param(
        [System.Drawing.Bitmap]$Bitmap,
        [int]$Y
    )

    $yClamped = [Math]::Max(0, [Math]::Min($Bitmap.Height - 1, $Y))
    $samples = 0
    $total = 0.0
    for ($x = 0; $x -lt $Bitmap.Width; $x += 8) {
        $pixel = $Bitmap.GetPixel($x, $yClamped)
        $total += ($pixel.R + $pixel.G + $pixel.B) / 3.0
        $samples++
    }
    if ($samples -eq 0) {
        return 0.0
    }
    return $total / $samples
}

function Remove-XemuMenuBarIfPresent {
    param([System.Drawing.Bitmap]$Bitmap)

    if ($Bitmap.Height -le 260) {
        return $Bitmap
    }

    $row0 = Get-RowMeanBrightness -Bitmap $Bitmap -Y 0
    $row10 = Get-RowMeanBrightness -Bitmap $Bitmap -Y 10
    $row20 = Get-RowMeanBrightness -Bitmap $Bitmap -Y 20
    $row24 = Get-RowMeanBrightness -Bitmap $Bitmap -Y 24

    $hasMenuBar = (($row0 -gt 20.0) -and ($row10 -gt 30.0) -and ($row20 -gt 20.0) -and ($row24 -lt ($row10 - 15.0)))
    if (-not $hasMenuBar) {
        return $Bitmap
    }

    $cropTop = 22
    $cropRect = New-Object System.Drawing.Rectangle 0, $cropTop, $Bitmap.Width, ($Bitmap.Height - $cropTop)
    $cropped = New-Object System.Drawing.Bitmap $cropRect.Width, $cropRect.Height
    $graphics = [System.Drawing.Graphics]::FromImage($cropped)
    try {
        $targetRect = New-Object System.Drawing.Rectangle 0, 0, $cropped.Width, $cropped.Height
        $graphics.DrawImage($Bitmap, $targetRect, $cropRect, [System.Drawing.GraphicsUnit]::Pixel)
    } finally {
        $graphics.Dispose()
    }
    $Bitmap.Dispose()
    return $cropped
}

function Copy-XemuClientToBitmap {
    param(
        [IntPtr]$Handle,
        [System.Drawing.Bitmap]$Target
    )

    $clientRect = New-Object XemuCapture+RECT
    [XemuCapture]::GetClientRect($Handle, [ref]$clientRect) | Out-Null
    $clientPoint = New-Object XemuCapture+POINT
    $clientPoint.X = 0
    $clientPoint.Y = 0
    [XemuCapture]::ClientToScreen($Handle, [ref]$clientPoint) | Out-Null

    $windowRect = New-Object XemuCapture+RECT
    [XemuCapture]::GetWindowRect($Handle, [ref]$windowRect) | Out-Null
    $windowWidth = [Math]::Max(1, $windowRect.Right - $windowRect.Left)
    $windowHeight = [Math]::Max(1, $windowRect.Bottom - $windowRect.Top)
    $srcX = [Math]::Max(0, $clientPoint.X - $windowRect.Left)
    $srcY = [Math]::Max(0, $clientPoint.Y - $windowRect.Top)

    $windowBitmap = New-Object System.Drawing.Bitmap $windowWidth, $windowHeight
    $windowGraphics = [System.Drawing.Graphics]::FromImage($windowBitmap)
    try {
        $hdc = $windowGraphics.GetHdc()
        try {
            $printed = [XemuCapture]::PrintWindow($Handle, $hdc, 0)
        } finally {
            $windowGraphics.ReleaseHdc($hdc)
        }

        if (-not $printed) {
            return $false
        }

        $targetGraphics = [System.Drawing.Graphics]::FromImage($Target)
        try {
            $sourceRect = New-Object System.Drawing.Rectangle $srcX, $srcY, $Target.Width, $Target.Height
            $targetRect = New-Object System.Drawing.Rectangle 0, 0, $Target.Width, $Target.Height
            $targetGraphics.DrawImage($windowBitmap, $targetRect, $sourceRect, [System.Drawing.GraphicsUnit]::Pixel)
        } finally {
            $targetGraphics.Dispose()
        }
        return $true
    } finally {
        $windowGraphics.Dispose()
        $windowBitmap.Dispose()
    }
}

function Copy-XemuScreenClientToBitmap {
    param(
        [IntPtr]$Handle,
        [System.Drawing.Bitmap]$Target
    )

    $clientRect = New-Object XemuCapture+RECT
    [XemuCapture]::GetClientRect($Handle, [ref]$clientRect) | Out-Null
    $clientPoint = New-Object XemuCapture+POINT
    $clientPoint.X = 0
    $clientPoint.Y = 0
    [XemuCapture]::ClientToScreen($Handle, [ref]$clientPoint) | Out-Null

    $targetGraphics = [System.Drawing.Graphics]::FromImage($Target)
    try {
        $targetGraphics.CopyFromScreen($clientPoint.X, $clientPoint.Y, 0, 0, $Target.Size)
    } finally {
        $targetGraphics.Dispose()
    }
}

function Capture-XemuIso {
    param(
        [string]$Iso,
        [string]$OutPath
    )

    & powershell -ExecutionPolicy Bypass -File $runXemu -Iso $Iso -NoStart -EmuRoot $EmuRoot | Out-Null
    if (-not (Test-Path $xemuExe)) {
        throw "xemu executable not found: $xemuExe"
    }
    if (-not (Test-Path $configPath)) {
        throw "xemu config not found: $configPath"
    }

    $args = @("-config_path", $configPath, "-dvd_path", $Iso)
    if (-not $NoSnapshot) {
        $args += "-snapshot"
    }
    $lastFailure = $null
    for ($launchAttempt = 1; $launchAttempt -le $LaunchAttempts; ++$launchAttempt) {
        $proc = $null
        try {
            Write-Host ("Starting xemu launch attempt {0}/{1}: {2}" -f $launchAttempt, $LaunchAttempts, $Iso)
            $proc = Start-Process -FilePath $xemuExe -ArgumentList $args -WorkingDirectory $xemuRoot -PassThru
            try {
                $handle = Get-MainWindowHandle -Process $proc
                Set-XemuCaptureWindow -Handle $handle
                Start-Sleep -Milliseconds ([int]($DelaySeconds * 1000))
                Close-XemuUpdateDialogs -Process $proc
                Set-XemuCaptureWindow -Handle $handle
                Start-Sleep -Milliseconds 750

                $rect = New-Object XemuCapture+RECT
                [XemuCapture]::GetClientRect($handle, [ref]$rect) | Out-Null

                $width = [Math]::Max(1, $rect.Right - $rect.Left)
                $height = [Math]::Max(1, $rect.Bottom - $rect.Top)
                $bitmap = New-Object System.Drawing.Bitmap $width, $height
                try {
                    New-Item -ItemType Directory -Force -Path (Split-Path -Parent $OutPath) | Out-Null
                    $debugDir = Split-Path -Parent $OutPath
                    $debugName = [System.IO.Path]::GetFileNameWithoutExtension($OutPath)
                    $attemptSuffix = if ($launchAttempt -eq 1) { "" } else { ".launch$launchAttempt" }
                    $debugBase = Join-Path $debugDir "$debugName$attemptSuffix"
                    $accepted = $false
                    $lastStats = $null
                    for ($attempt = 1; $attempt -le 5; ++$attempt) {
                        Close-XemuUpdateDialogs -Process $proc
                        Set-XemuCaptureWindow -Handle $handle
                        Start-Sleep -Milliseconds 150
                        Copy-XemuScreenClientToBitmap -Handle $handle -Target $bitmap
                        $lastStats = Get-CaptureStats -Bitmap $bitmap
                        if (Test-CaptureLooksLikeFramebuffer -Bitmap $bitmap) {
                            $accepted = $true
                            break
                        }
                        if ($DebugRejectedCaptures -and $attempt -eq 1) {
                            $bitmap.Save("$debugBase.rejected-screen.png", [System.Drawing.Imaging.ImageFormat]::Png)
                        }

                        if ($DebugRejectedCaptures -and $attempt -eq 1 -and (Copy-XemuClientToBitmap -Handle $handle -Target $bitmap)) {
                            $bitmap.Save("$debugBase.rejected-printwindow.png", [System.Drawing.Imaging.ImageFormat]::Png)
                        }

                        Start-Sleep -Milliseconds 1000
                    }
                    if (-not $accepted) {
                        if ($DebugRejectedCaptures) {
                            $bitmap.Save("$debugBase.rejected-client.png", [System.Drawing.Imaging.ImageFormat]::Png)

                            $windowRect = New-Object XemuCapture+RECT
                            [XemuCapture]::GetWindowRect($handle, [ref]$windowRect) | Out-Null
                            $windowWidth = [Math]::Max(1, $windowRect.Right - $windowRect.Left)
                            $windowHeight = [Math]::Max(1, $windowRect.Bottom - $windowRect.Top)
                            $windowBitmap = New-Object System.Drawing.Bitmap $windowWidth, $windowHeight
                            $windowGraphics = [System.Drawing.Graphics]::FromImage($windowBitmap)
                            try {
                                $windowGraphics.CopyFromScreen($windowRect.Left, $windowRect.Top, 0, 0, $windowBitmap.Size)
                                $windowBitmap.Save("$debugBase.rejected-window.png", [System.Drawing.Imaging.ImageFormat]::Png)
                            } finally {
                                $windowGraphics.Dispose()
                                $windowBitmap.Dispose()
                            }
                        }
                        $lastFailure = ("Capture does not look like a rendered xemu framebuffer on launch attempt {0}/{1}; refusing to save it. mean_brightness={2:N2} non_dark_ratio={3:N4} corner_bright_neutral_ratio={4:N4} upper_right_bright_neutral_ratio={5:N4}" -f $launchAttempt, $LaunchAttempts, $lastStats.mean_brightness, $lastStats.non_dark_ratio, $lastStats.corner_bright_neutral_ratio, $lastStats.upper_right_bright_neutral_ratio)
                    } else {
                        $bitmap = Remove-XemuMenuBarIfPresent -Bitmap $bitmap
                        $bitmap.Save($OutPath, [System.Drawing.Imaging.ImageFormat]::Png)
                        Write-Host "Captured $OutPath"
                        return
                    }
                } finally {
                    $bitmap.Dispose()
                }
            } finally {
                if (-not $proc.HasExited) {
                    Stop-Process -Id $proc.Id -Force
                }
            }
        } catch {
            $lastFailure = $_.Exception.Message
            if ($proc -ne $null -and -not $proc.HasExited) {
                Stop-Process -Id $proc.Id -Force
            }
        }
        if ($launchAttempt -lt $LaunchAttempts) {
            Write-Warning "$lastFailure; retrying with a fresh xemu process"
            Start-Sleep -Milliseconds 1500
        }
    }
    if ([string]::IsNullOrWhiteSpace($lastFailure)) {
        $lastFailure = "Capture failed without a detailed rejection reason"
    }
    throw $lastFailure
}

function Get-NeHeAppNumber {
    param(
        [string]$SetName,
        [int]$Lesson
    )

    if ($SetName -eq "nxgl") {
        if ($Lesson -le 12) {
            return 110 + $Lesson
        }
        return 200 + $Lesson
    }

    if ($Lesson -le 12) {
        return 122 + $Lesson
    }
    return 300 + $Lesson
}

$sets = switch ($Set) {
    "nxgl" { @("nxgl") }
    "pb" { @("pb") }
    default { @("nxgl","pb") }
}

foreach ($setName in $sets) {
    foreach ($lessonToken in $Lessons) {
        foreach ($lessonPart in ($lessonToken -split ",")) {
            if ([string]::IsNullOrWhiteSpace($lessonPart)) {
                continue
            }
            $lesson = [int]$lessonPart.Trim()
            if ($lesson -lt 1 -or $lesson -gt $lessonLabels.Count) {
                throw "Unsupported lesson number: $lesson"
            }

            $label = $lessonLabels[$lesson - 1]
            $appNumber = Get-NeHeAppNumber $setName $lesson
            if ($setName -eq "nxgl") {
                if ($lesson -le 12) {
                    $iso = Join-Path $isoRoot ("xbnehe_{0}_nxgl_{1:00}_{2}.iso" -f $appNumber, $lesson, $label)
                } else {
                    $iso = Join-Path $isoRoot ("xbnehe_{0}_nehe_nxgl_{1:00}_{2}.iso" -f $appNumber, $lesson, $label)
                }
            } else {
                if ($lesson -le 12) {
                    $iso = Join-Path $isoRoot ("xbnehe_{0}_pb_{1:00}_{2}.iso" -f $appNumber, $lesson, $label)
                } else {
                    $iso = Join-Path $isoRoot ("xbnehe_{0}_nehe_pb_{1:00}_{2}.iso" -f $appNumber, $lesson, $label)
                }
            }

            if (-not (Test-Path $iso)) {
                throw "Missing ISO: $iso"
            }
            $out = Join-Path $captureRoot (Join-Path $setName ("nehe_{0}_{1:00}.png" -f $setName, $lesson))
            Capture-XemuIso -Iso $iso -OutPath $out
        }
    }
}
