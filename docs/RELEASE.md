# Release Checklist

## Source Hygiene

- No generated `.inl`, `.obj`, `.d`, `.exe`, `.xbe`, or `.iso` files are staged.
- `LICENSE` is MIT and present at the repository root.
- `README.md` and `docs/BUILDING.md` match the current expected checkout layout.
- The NXGL sibling checkout builds at least one NXGL lesson.

## Build

From MSYS2:

```sh
export MSYSTEM=MINGW64
export NXDK_DIR=/c/path/to/xb_homebrew/.nxdk
export PATH="$NXDK_DIR/bin:$NXDK_DIR/tools/cg/win:/usr/bin:$PATH"

make clean
make
./tools/collect_release_isos.sh
```

Expected release output:

- 96 ISO files in `dist/release/nehe/isos`
- 96 XBE folders in `dist/release/nehe/xbes`

## Visual Check

Use the verified visual wrapper to compare Windows reference captures, NXGL
xemu captures, and PBKit xemu captures:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\verify_nehe_visuals.ps1 -SkipBuild
```

The xemu capture helper runs lessons serially and rejects blank or obvious
desktop/window-manager captures before writing the PNGs. The capture helper also
crops xemu's menu bar when it appears in client-area captures.
