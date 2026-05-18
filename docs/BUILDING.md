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
