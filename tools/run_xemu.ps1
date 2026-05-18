param(
    [string]$Iso = "",
    [switch]$FullScreen,
    [switch]$NoSnapshot,
    [switch]$NoStart
)

$ErrorActionPreference = "Stop"

$Workspace = Split-Path -Parent $PSScriptRoot
$EmuRoot = Join-Path $Workspace "Xbox-Emulator-Files"
$XemuDir = Join-Path $EmuRoot "xemu"
$XemuExe = Join-Path $XemuDir "xemu.exe"
$DownloadZip = Join-Path $EmuRoot "xemu-0.8.134-windows-x86_64.zip"
$DownloadUrl = "https://github.com/xemu-project/xemu/releases/latest/download/xemu-0.8.134-windows-x86_64.zip"

$BiosPath = Join-Path $EmuRoot "bios\Complex_4627.bin"
$McpxPath = Join-Path $EmuRoot "mcpx\mcpx_1.0.bin"
$HddPath = Join-Path $EmuRoot "hdd\xbox_hdd.qcow2"
$EepromDir = Join-Path $EmuRoot "eeprom"
$EepromPath = Join-Path $EepromDir "eeprom.bin"
$ConfigPath = Join-Path $XemuDir "xemu-local.toml"

if (!(Test-Path $XemuExe)) {
    New-Item -ItemType Directory -Path $XemuDir -Force | Out-Null
    if (!(Test-Path $DownloadZip)) {
        Invoke-WebRequest -Uri $DownloadUrl -OutFile $DownloadZip
    }
    Expand-Archive -Path $DownloadZip -DestinationPath $XemuDir -Force
}

foreach ($required in @($BiosPath, $McpxPath, $HddPath)) {
    if (!(Test-Path $required)) {
        throw "Missing required xemu file: $required"
    }
}

if (!(Test-Path $EepromDir)) {
    New-Item -ItemType Directory -Path $EepromDir -Force | Out-Null
}

$DefaultAppDataEeprom = Join-Path $env:APPDATA "xemu\xemu\eeprom.bin"
if (!(Test-Path $EepromPath) -and (Test-Path $DefaultAppDataEeprom)) {
    Copy-Item -LiteralPath $DefaultAppDataEeprom -Destination $EepromPath
}

if ([string]::IsNullOrWhiteSpace($Iso)) {
    $Iso = Join-Path $Workspace "dist\release\nehe\isos\xbnehe_111_nxgl_01_window.iso"
} elseif (![System.IO.Path]::IsPathRooted($Iso)) {
    $Iso = Join-Path $Workspace $Iso
}

if (!(Test-Path $Iso)) {
    throw "ISO not found: $Iso"
}

$Config = @"
[general]
show_welcome = false
skip_boot_anim = true

[display.window]
fullscreen_on_startup = $($FullScreen.IsPresent.ToString().ToLowerInvariant())

[sys]
mem_limit = '64'

[sys.files]
bootrom_path = '$McpxPath'
flashrom_path = '$BiosPath'
eeprom_path = '$EepromPath'
hdd_path = '$HddPath'
"@

Set-Content -Path $ConfigPath -Value $Config -Encoding ASCII

$Arguments = @("-config_path", $ConfigPath, "-dvd_path", $Iso)
if (!$NoSnapshot) {
    $Arguments += "-snapshot"
}
if ($FullScreen) {
    $Arguments += "-full-screen"
}

Write-Host "xemu: $XemuExe"
Write-Host "config: $ConfigPath"
Write-Host "iso: $Iso"
Write-Host "args: $($Arguments -join ' ')"

if ($NoStart) {
    return
}

Start-Process -FilePath $XemuExe -ArgumentList $Arguments -WorkingDirectory $XemuDir
