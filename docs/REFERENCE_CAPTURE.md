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

The xemu capture path uses the emulator window handle and retries a failed
capture in a fresh xemu process. If a run is flaky, increase
`-LaunchAttempts` and add `-DebugRejectedCaptures` to save rejected
PrintWindow/screen frames next to the intended output.

The wrapper writes fresh xemu captures to
`dist/nehe_reference/captures/xemu_verified` and comparison sheets to
`dist/nehe_reference/captures/compare`.

The Xbox build/capture helpers prefer the devkitPro MSYS2 shell at
`C:\devkitPro\msys2\usr\bin\bash.exe`. A generic `C:\msys64` shell may have
the right executable name but the wrong nxdk/devkit environment.

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
If a long sweep is interrupted, rerun the same command with
`-ResumeExisting` to skip fixed-time PNGs already present in that folder.

For the standard cube/display-list regression set, use the regression wrapper.
It runs a fixed-time xemu sweep, stages the frames into a normal capture set,
and generates strict verified comparison sheets:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\verify_nehe_regression_set.ps1
```

By default this checks lessons `5,6,7,8,12` at `2000 ms`, writes staged
captures to `dist/nehe_reference/captures/xemu_regression_2000`, and refreshes
the comparison artifacts in `dist/nehe_reference/captures/compare`.

To regenerate comparison sheets from an existing fixed-time sweep without
recapturing xemu frames, pass the original sweep label:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\verify_nehe_regression_set.ps1 -SkipSweep -Label regression_05_06_07_08_12_2000ms
```

For lessons that do not yet have Windows reference captures, use `-XemuOnly`
to compare NXGL and PBKit captures directly:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\verify_nehe_regression_set.ps1 -Lessons 13,14,15,16,17,18,19 -TimeMs 2500 -CaptureSetName xemu_regression_13_19_2500 -XemuOnly -ResumeExisting
```

Captured output is written under `dist/nehe_reference/captures`.
