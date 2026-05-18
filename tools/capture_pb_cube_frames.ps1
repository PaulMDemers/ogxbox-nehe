param(
    [int[]]$Lessons = @(6,7,8),
    [int[]]$Times = @(0,1000,2000,3000,4000,5000,6000,7000,8000),
    [double]$DelaySeconds = 5.0,
    [string]$Label = "before_edge_fix"
)
$ErrorActionPreference = "Stop"
$repo = Resolve-Path (Join-Path $PSScriptRoot "..")
$repoMsys = ($repo.Path -replace '\\','/')
if ($repoMsys -match '^([A-Za-z]):/(.*)$') {
    $repoMsys = "/$($matches[1].ToLowerInvariant())/$($matches[2])"
}
$nxdk = Resolve-Path (Join-Path $repo "..\.nxdk")
$nxdkMsys = ($nxdk.Path -replace '\\','/')
if ($nxdkMsys -match '^([A-Za-z]):/(.*)$') {
    $nxdkMsys = "/$($matches[1].ToLowerInvariant())/$($matches[2])"
}
$outRoot = Join-Path $repo (Join-Path "dist\nehe_reference\captures\xemu_frames" $Label)
$labels = @("window","first_polygons","color","rotation","3d_shapes","texture_mapping","filters_lighting","blending","moving_bitmaps","3d_world","flag_effect","display_lists")
$apps = @{ 6="128_nehe_pb_06_texture_mapping"; 7="129_nehe_pb_07_filters_lighting"; 8="130_nehe_pb_08_blending"; 12="134_nehe_pb_12_display_lists" }
$msys = "C:\devkitPro\msys2\usr\bin\bash.exe"
foreach ($lesson in $Lessons) {
    foreach ($time in $Times) {
        $app = $apps[$lesson]
        if (-not $app) { throw "No PB app mapping for lesson $lesson" }
        $cmd = "export MSYSTEM=MINGW64; export NXDK_DIR=$nxdkMsys; export PATH=$nxdkMsys/bin:$nxdkMsys/tools/cg/win:/usr/bin:`$PATH; cd $repoMsys && make -C $app clean >/dev/null && make -C $app NEHE_CAPTURE_FIXED_TIME_MS=$time >/dev/null && ./tools/collect_release_isos.sh >/dev/null"
        & $msys -lc $cmd
        $capDir = Join-Path $outRoot ("pb_{0:D2}" -f $lesson)
        New-Item -ItemType Directory -Force -Path $capDir | Out-Null
        & powershell -ExecutionPolicy Bypass -File (Join-Path $repo "tools\capture_nehe_xemu.ps1") -Set pb -Lessons $lesson -DelaySeconds $DelaySeconds | Out-Null
        $src = Join-Path $repo ("dist\nehe_reference\captures\xemu\pb\nehe_pb_{0:D2}.png" -f $lesson)
        $dst = Join-Path $capDir ("pb_{0:D2}_{1:D5}ms.png" -f $lesson,$time)
        Copy-Item -Force $src $dst
        Write-Host "captured $dst"
    }
}
