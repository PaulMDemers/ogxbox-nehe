param(
    [int[]]$Lessons = @(6,7,8),
    [int[]]$Times = @(0,1000,2000,3000,4000,5000,6000,7000,8000),
    [double]$DelaySeconds = 5.0,
    [string]$Label = "before_edge_fix"
)

$ErrorActionPreference = "Stop"

& (Join-Path $PSScriptRoot "capture_nehe_frame_sweep.ps1") `
    -Set pb `
    -Lessons $Lessons `
    -Times $Times `
    -DelaySeconds $DelaySeconds `
    -Label $Label
