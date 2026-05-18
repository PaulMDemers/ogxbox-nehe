param(
    [ValidateSet("nxgl","pb","all")]
    [string]$Set = "all",
    [string[]]$Lessons = @("1","2","3","4","5","6","7","8","9","10","11","12"),
    [double]$DelaySeconds = 12.0
)

$ErrorActionPreference = "Stop"

$repo = Resolve-Path (Join-Path $PSScriptRoot "..")
$isoRoot = Join-Path $repo "dist\release\nehe\isos"
$captureRoot = Join-Path $repo "dist\nehe_reference\captures\xemu"
$xemuRoot = Join-Path $repo "Xbox-Emulator-Files\xemu"
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

function Test-CaptureLooksLikeXemu {
    param([System.Drawing.Bitmap]$Bitmap)

    $samples = 0
    $total = 0.0
    for ($y = 0; $y -lt $Bitmap.Height; $y += 16) {
        for ($x = 0; $x -lt $Bitmap.Width; $x += 16) {
            $pixel = $Bitmap.GetPixel($x, $y)
            $total += ($pixel.R + $pixel.G + $pixel.B) / 3.0
            $samples++
        }
    }

    if ($samples -eq 0) {
        return $false
    }

    return (($total / $samples) -lt 130.0)
}

function Capture-XemuIso {
    param(
        [string]$Iso,
        [string]$OutPath
    )

    & powershell -ExecutionPolicy Bypass -File $runXemu -Iso $Iso -NoStart | Out-Null
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
            $graphics.CopyFromScreen($point.X, $point.Y, 0, 0, $bitmap.Size)
            New-Item -ItemType Directory -Force -Path (Split-Path -Parent $OutPath) | Out-Null
            if (-not (Test-CaptureLooksLikeXemu -Bitmap $bitmap)) {
                Set-XemuCaptureWindow -Handle $handle
                Start-Sleep -Milliseconds 750
                $graphics.CopyFromScreen($point.X, $point.Y, 0, 0, $bitmap.Size)
            }
            if (-not (Test-CaptureLooksLikeXemu -Bitmap $bitmap)) {
                throw "Capture does not look like the xemu framebuffer; refusing to save desktop/window-manager content."
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
