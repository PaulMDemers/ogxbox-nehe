# Architecture

The repository contains two standalone lesson families.

## NXGL Lessons

`111_nehe_nxgl_*` through `122_nehe_nxgl_*` are OpenGL-style ports. Each lesson
defines a standalone lesson index and includes the shared source in
`110_nehe_nxgl_demo_disk/main.c`.

These lessons link against NXGL through a sibling checkout:

```make
NXGL_DIR ?= $(CURDIR)/../../nxgl
include $(NXGL_DIR)/nxgl.mk
```

## PBKit Lessons

`123_nehe_pb_*` through `134_nehe_pb_*` render directly through the native PBKit
helper layer in `common_nehe/`. They are useful as a visual baseline when
comparing NXGL behavior.

## Shared Code

- `common_nehe/` contains lesson data, native helper code, generated asset
  headers, capture-time hooks, and shader sources.
- `common3d/` contains shared shader sources used by the native renderer path.
- `tools/` contains build, capture, xemu, and comparison helpers.

## Lesson Numbering

The default release includes 24 standalone builds:

- NXGL lessons 1-12: directories `111` through `122`.
- PBKit lessons 1-12: directories `123` through `134`.

The `110_nehe_nxgl_demo_disk` directory is retained only as shared source for
the standalone NXGL lesson wrappers.
