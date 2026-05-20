# Release Layout

The source tree keeps the historical numeric prefixes so existing build scripts
and report names stay stable, but the root `Makefile` now groups apps by release
role.

## Build Groups

- `make all` builds every standalone NeHe lesson.
- `make release` builds the lessons and collects release artifacts.
- `make print-nehe-apps` prints every app included in the release.
- `make print-nehe-nxgl-apps` and `make print-nehe-pb-apps` print one renderer
  family at a time.

## ISO Collection

Use the release collector when preparing a staged release:

```sh
./tools/collect_release_isos.sh
```

That produces:

- `dist/release/nehe`

The NeHe release group contains:

- `isos/` with the newest built ISO for each app.
- `xbes/<app>/default.xbe` with the matching built XBE for direct launch.

## NeHe Disks

- `110_nehe_nxgl_demo_disk` is retained as shared source for the standalone
  NXGL lesson wrappers.
- `111` through `122` and `213` through `248` are standalone NXGL lesson disks.
- `123` through `134` and `313` through `348` are standalone PBKit baseline
  lesson disks.

The lesson rendering code lives in `common_nehe/nehe_lessons.c` for PBKit and
`110_nehe_nxgl_demo_disk/main.c` for NXGL. The classic NeHe set ends at lesson
48; supplemental demos should use a separate naming scheme.
