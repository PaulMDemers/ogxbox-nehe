# Reference Capture Workflow

The tools in this repository support visual comparison between three render
paths:

- Windows reference builds of the original tutorial behavior.
- NXGL builds running in xemu.
- PBKit/native builds running in xemu.

## Build Windows References

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build_nehe_reference.ps1
```

## Capture Windows References

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\capture_nehe_reference.ps1
```

## Capture xemu Frames

The preferred path is the verified wrapper. It can build, capture NXGL/PBKit
frames serially in xemu, generate comparison sheets, and fail if a captured
Xbox frame is blank:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\verify_nehe_visuals.ps1
```

Use comma-separated lesson lists when narrowing a run:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\verify_nehe_visuals.ps1 -SkipBuild -Lessons 5,6,7,12
```

The wrapper writes fresh xemu captures to
`dist/nehe_reference/captures/xemu_verified` and comparison sheets to
`dist/nehe_reference/captures/compare`.

To run the lower-level capture helper directly, build and collect the Xbox
release artifacts first:

```sh
make
./tools/collect_release_isos.sh
```

Then capture:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\capture_nehe_xemu.ps1 -Set all
```

## Generate Comparison Sheets

The verified generator records per-image brightness/content bounds and
Windows-vs-xemu comparison metrics in
`nehe_verified_compare_metrics.json`:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\generate_nehe_verified_compare.ps1
```

The older fixed-sheet generator is still available:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\generate_nehe_compare.ps1
```

## Capture Rotation Sweeps

For rotation-sensitive artifacts, rebuild selected lessons at fixed tutorial
times and capture each frame serially:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\capture_nehe_frame_sweep.ps1 -Set all -Lessons 5,6,7,8,12 -Times 0,1000,2000,3000,4000,5000,6000
```

The sweep writes PNGs and a `manifest.json` under
`dist/nehe_reference/captures/xemu_frames/<label>`.

Captured output is written under `dist/nehe_reference/captures`.
