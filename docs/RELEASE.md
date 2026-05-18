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

- 24 ISO files in `dist/release/nehe/isos`
- 24 XBE folders in `dist/release/nehe/xbes`

## Visual Check

Use the capture helpers in `tools/` to compare Windows reference captures, NXGL
xemu captures, and PBKit xemu captures.
