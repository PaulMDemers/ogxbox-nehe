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

Build and collect the Xbox release artifacts first:

```sh
make
./tools/collect_release_isos.sh
```

Then capture:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\capture_nehe_xemu.ps1 -Set all
```

## Generate Comparison Sheets

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\generate_nehe_compare.ps1
```

Captured output is written under `dist/nehe_reference/captures`.
