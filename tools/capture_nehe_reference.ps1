param(
    [string[]]$Lessons = @("1","2","3","4","5","6","7","8","9","10","11","12"),
    [double]$DelaySeconds = 2.0
)

$ErrorActionPreference = "Stop"

$repo = Resolve-Path (Join-Path $PSScriptRoot "..")
$buildRoot = Join-Path $repo "dist\nehe_reference\build"
$captureRoot = Join-Path $repo "dist\nehe_reference\captures\windows"
New-Item -ItemType Directory -Force -Path $captureRoot | Out-Null

Add-Type -AssemblyName System.Drawing
Add-Type @"
using System;
using System.Runtime.InteropServices;

public static class Win32Capture {
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
    public static extern bool SetProcessDPIAware();

    public static readonly IntPtr HWND_TOPMOST = new IntPtr(-1);
    public const uint SWP_SHOWWINDOW = 0x0040;
    public const int SW_RESTORE = 9;
}
"@

[Win32Capture]::SetProcessDPIAware() | Out-Null

function Get-MainWindowHandle {
    param([System.Diagnostics.Process]$Process)

    for ($i = 0; $i -lt 100; $i++) {
        $Process.Refresh()
        if ($Process.MainWindowHandle -ne [IntPtr]::Zero) {
            return $Process.MainWindowHandle
        }
        Start-Sleep -Milliseconds 100
    }
    throw "No window handle for process $($Process.Id)"
}

foreach ($lessonToken in $Lessons) {
    foreach ($lessonPart in ($lessonToken -split ",")) {
        if ([string]::IsNullOrWhiteSpace($lessonPart)) {
            continue
        }
        $lesson = [int]$lessonPart.Trim()
        $name = "lesson$lesson"
        $dir = Join-Path $buildRoot $name
        $exe = Join-Path $dir "$name.exe"
        if (-not (Test-Path $exe)) {
            throw "Missing reference executable: $exe"
        }

        $proc = Start-Process -FilePath $exe -WorkingDirectory $dir -PassThru
        try {
            $handle = Get-MainWindowHandle -Process $proc
            [Win32Capture]::ShowWindow($handle, [Win32Capture]::SW_RESTORE) | Out-Null
            [Win32Capture]::SetWindowPos($handle, [Win32Capture]::HWND_TOPMOST, 60, 60, 660, 520, [Win32Capture]::SWP_SHOWWINDOW) | Out-Null
            Start-Sleep -Milliseconds ([int]($DelaySeconds * 1000))

            $rect = New-Object Win32Capture+RECT
            [Win32Capture]::GetWindowRect($handle, [ref]$rect) | Out-Null
            $width = [Math]::Max(1, $rect.Right - $rect.Left)
            $height = [Math]::Max(1, $rect.Bottom - $rect.Top)
            $bitmap = New-Object System.Drawing.Bitmap $width, $height
            $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
            try {
                $graphics.CopyFromScreen($rect.Left, $rect.Top, 0, 0, $bitmap.Size)
                $outPath = Join-Path $captureRoot ("nehe_ref_{0:00}.png" -f $lesson)
                $bitmap.Save($outPath, [System.Drawing.Imaging.ImageFormat]::Png)
                Write-Host "Captured $outPath"
            } finally {
                $graphics.Dispose()
                $bitmap.Dispose()
            }
        } finally {
            if (-not $proc.HasExited) {
                $proc.CloseMainWindow() | Out-Null
                Start-Sleep -Milliseconds 250
            }
            if (-not $proc.HasExited) {
                Stop-Process -Id $proc.Id -Force
            }
        }
    }
}
