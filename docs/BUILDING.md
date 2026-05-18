# Building The NeHe Demos

## Requirements

- Windows with MSYS2.
- A working nxdk checkout.
- NXGL checked out beside this repository for the NXGL lesson family.

Expected layout:

```text
xb_homebrew/
  .nxdk/
  nxgl/
  ogxbox-nehe/
```

Example MSYS2 setup:

```sh
export MSYSTEM=MINGW64
export NXDK_DIR=/c/path/to/xb_homebrew/.nxdk
export PATH="$NXDK_DIR/bin:$NXDK_DIR/tools/cg/win:/usr/bin:$PATH"
```

## Build Everything

```sh
make
```

## Build One Lesson

```sh
make -C 116_nehe_nxgl_06_texture_mapping
make -C 128_nehe_pb_06_texture_mapping
```

The PBKit lesson family is kept as a reference baseline. Build matching NXGL and
PBKit lesson numbers when checking whether a rendering issue belongs to NXGL or
to shared lesson data:

```sh
make -C 117_nehe_nxgl_07_filters_lighting
make -C 129_nehe_pb_07_filters_lighting
```

## Scaffold A New Lesson

Use the scaffold helper after adding the lesson implementation to
`common_nehe`:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\new_nehe_lesson.ps1 -Lesson 13 -UpdateMakefile
```

Lessons after 12 use separate numbering bands by default (`2xx` for NXGL and
`3xx` for PBKit) so new NXGL lesson numbers do not collide with the released
PBKit lesson 1-12 directories.

## Release Artifacts

```sh
make release
```

or:

```sh
./tools/collect_release_isos.sh
```

Artifacts are copied to `dist/release/nehe`.

## Generated Files

The build creates `.inl`, `.obj`, `.d`, `.exe`, `.xbe`, and `.iso` outputs.
These are ignored and should not be committed.
