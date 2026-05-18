param(
    [ValidateSet("nxgl","pb","all")]
    [string]$Set = "all",
    [string[]]$Lessons = @("1","2","3","4","5","6","7","8","9","10","11","12"),
    [double]$DelaySeconds = 12.0,
    [string]$OutputSetName = "xemu",
    [string]$EmuRoot = "",
    [switch]$DebugRejectedCaptures
)

$ErrorActionPreference = "Stop"

$repo = Resolve-Path (Join-Path $PSScriptRoot "..")
$isoRoot = Join-Path $repo "dist\release\nehe\isos"
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

    public static readonly IntPtr HWND_TOPMOST = new IntPtr(-1);
    public const uint SWP_SHOWWINDOW = 0x0040;
    public const int SW_RESTORE = 9;
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

function Get-CaptureStats {
    param([System.Drawing.Bitmap]$Bitmap)

    $samples = 0
    $total = 0.0
    $nonDark = 0
    for ($y = 0; $y -lt $Bitmap.Height; $y += 16) {
        for ($x = 0; $x -lt $Bitmap.Width; $x += 16) {
            $pixel = $Bitmap.GetPixel($x, $y)
            $value = ($pixel.R + $pixel.G + $pixel.B) / 3.0
            $total += $value
            if ($value -gt 12.0) {
                $nonDark++
            }
            $samples++
        }
    }

    if ($samples -eq 0) {
        $samples = 1
    }

    return [ordered]@{
        mean_brightness = $total / $samples
        non_dark_ratio = $nonDark / $samples
    }
}

function Test-CaptureLooksLikeFramebuffer {
    param([System.Drawing.Bitmap]$Bitmap)

    $stats = Get-CaptureStats -Bitmap $Bitmap
    return (($stats.mean_brightness -lt 130.0) -and ($stats.non_dark_ratio -gt 0.002))
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

    $args = @("-config_path", $configPath, "-dvd_path", $Iso, "-snapshot")
    $proc = Start-Process -FilePath $xemuExe -ArgumentList $args -WorkingDirectory $xemuRoot -PassThru
    try {
        $handle = Get-MainWindowHandle -Process $proc
        Set-XemuCaptureWindow -Handle $handle
        Start-Sleep -Milliseconds ([int]($DelaySeconds * 1000))
        Set-XemuCaptureWindow -Handle $handle
        Start-Sleep -Milliseconds 750

        $rect = New-Object XemuCapture+RECT
        [XemuCapture]::GetClientRect($handle, [ref]$rect) | Out-Null
        $point = New-Object XemuCapture+POINT
        $point.X = 0
        $point.Y = 0
        [XemuCapture]::ClientToScreen($handle, [ref]$point) | Out-Null

        $width = [Math]::Max(1, $rect.Right - $rect.Left)
        $height = [Math]::Max(1, $rect.Bottom - $rect.Top)
        $bitmap = New-Object System.Drawing.Bitmap $width, $height
        $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
        try {
            New-Item -ItemType Directory -Force -Path (Split-Path -Parent $OutPath) | Out-Null
            $accepted = $false
            $lastStats = $null
            for ($attempt = 1; $attempt -le 5; ++$attempt) {
                $graphics.CopyFromScreen($point.X, $point.Y, 0, 0, $bitmap.Size)
                $lastStats = Get-CaptureStats -Bitmap $bitmap
                if (Test-CaptureLooksLikeFramebuffer -Bitmap $bitmap) {
                    $accepted = $true
                    break
                }
                Set-XemuCaptureWindow -Handle $handle
                Start-Sleep -Milliseconds 1000
            }
            if (-not $accepted) {
                if ($DebugRejectedCaptures) {
                    $debugDir = Split-Path -Parent $OutPath
                    $debugName = [System.IO.Path]::GetFileNameWithoutExtension($OutPath)
                    $debugBase = Join-Path $debugDir $debugName
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
                throw ("Capture does not look like a rendered xemu framebuffer; refusing to save it. mean_brightness={0:N2} non_dark_ratio={1:N4}" -f $lastStats.mean_brightness, $lastStats.non_dark_ratio)
            }
            $bitmap.Save($OutPath, [System.Drawing.Imaging.ImageFormat]::Png)
            Write-Host "Captured $OutPath"
        } finally {
            $graphics.Dispose()
            $bitmap.Dispose()
        }
    } finally {
        if (-not $proc.HasExited) {
            Stop-Process -Id $proc.Id -Force
        }
    }
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
            if ($lesson -lt 1 -or $lesson -gt 12) {
                throw "Unsupported lesson number: $lesson"
            }

            if ($setName -eq "nxgl") {
                $appNumber = 110 + $lesson
                $label = @(
                    "window","first_polygons","color","rotation","3d_shapes","texture_mapping",
                    "filters_lighting","blending","moving_bitmaps","3d_world","flag_effect","display_lists"
                )[$lesson - 1]
                $iso = Join-Path $isoRoot ("xbnehe_{0}_nxgl_{1:00}_{2}.iso" -f $appNumber, $lesson, $label)
            } else {
                $appNumber = 122 + $lesson
                $label = @(
                    "window","first_polygons","color","rotation","3d_shapes","texture_mapping",
                    "filters_lighting","blending","moving_bitmaps","3d_world","flag_effect","display_lists"
                )[$lesson - 1]
                $iso = Join-Path $isoRoot ("xbnehe_{0}_pb_{1:00}_{2}.iso" -f $appNumber, $lesson, $label)
            }

            if (-not (Test-Path $iso)) {
                throw "Missing ISO: $iso"
            }
            $out = Join-Path $captureRoot (Join-Path $setName ("nehe_{0}_{1:00}.png" -f $setName, $lesson))
            Capture-XemuIso -Iso $iso -OutPath $out
        }
    }
}
