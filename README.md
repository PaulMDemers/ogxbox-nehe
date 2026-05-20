# Original Xbox NeHe Demos

Standalone ports of the classic NeHe OpenGL tutorials for the original Xbox.
The collection exists both as a demo set and as a visual compatibility harness
for NXGL.

This repository contains two implementations of the classic NeHe tutorial
sequence, lessons 1 through 48:

- `111_nehe_nxgl_*` through `122_nehe_nxgl_*`, then `213_nehe_nxgl_*`
  through `248_nehe_nxgl_*`: GL-style ports that build on NXGL from a sibling
  checkout.
- `123_nehe_pb_*` through `134_nehe_pb_*`, then `313_nehe_pb_*` through
  `348_nehe_pb_*`: PBKit/native ports used as a direct renderer comparison and
  visual baseline.

`110_nehe_nxgl_demo_disk` remains as shared source for the NXGL standalone
lessons, but it is not part of the default release build.

## Expected Checkout Layout

```text
xb_homebrew/
  nxgl/
  ogxbox-nehe/
  .nxdk/
```

The NXGL demos default to `NXGL_DIR=$(CURDIR)/../../nxgl` from each lesson
directory, so a sibling checkout beside this repository is enough. Override
`NXGL_DIR` if you keep it somewhere else.

## Build

From MSYS2 with nxdk configured:

```sh
make
./tools/collect_release_isos.sh
```

Release artifacts are collected under:

- `dist/release/nehe/isos`
- `dist/release/nehe/xbes`

The default release contains 96 standalone builds: 48 NXGL lessons and 48 PBKit
lessons.

## Documentation

- [`docs/BUILDING.md`](docs/BUILDING.md): build environment and common commands.
- [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md): repository layout and lesson
  families.
- [`docs/RELEASE.md`](docs/RELEASE.md): release packaging and verification.
- [`docs/REFERENCE_CAPTURE.md`](docs/REFERENCE_CAPTURE.md): Windows/xemu visual
  comparison workflow.
- [`docs/PBKIT_BASELINE.md`](docs/PBKIT_BASELINE.md): how the PBKit lesson
  family is used as the native-renderer reference.
- [`docs/release_layout.md`](docs/release_layout.md): generated artifact layout.

## Attribution

The demos are based on the historic NeHe OpenGL tutorial sequence. This project
contains original Xbox ports and compatibility scaffolding. The classic
tutorial sequence ends at lesson 48, and the tutorial material remains credited
to NeHe and its original authors.

## License

This repository is released under the MIT License. See [`LICENSE`](LICENSE).
