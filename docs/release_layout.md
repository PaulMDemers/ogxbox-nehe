# Release Layout

The source tree keeps the historical numeric prefixes so existing build scripts
and report names stay stable, but the root `Makefile` now groups apps by release
role.

## Build Groups

- `make release` builds user-facing smoke tests, games, utilities, and NeHe demos.
- `make validation` builds the consolidated NXGL autorun validation suite.
- `make oneoffs` builds focused diagnostics and individual NXGL probe disks.
- `make all` still builds everything.

## ISO Collection

Use the flat collector when you want every built ISO in one directory:

```sh
make collect-isos
```

Use the release collector when preparing a staged release:

```sh
make collect-release-isos
```

That produces:

- `dist/release/smoke-tests`
- `dist/release/games`
- `dist/release/utilities`
- `dist/release/nehe`
- `dist/release/validation`
- `dist/release/one-offs`
- `dist/release/one-off-probes`

Each group contains:

- `isos/` with the newest built ISO for each app.
- `xbes/<app>/default.xbe` with the matching built XBE for direct launch.

## NeHe Disks

- `24_nehe_native_suite` remains the auto-cycling regression disk.
- `109_nehe_demo_disk` is the manual demo disk. D-pad left/right, left/right
  stick, White/Black, or LT/RT move between the current NeHe lessons.

The lesson rendering code lives in `common_nehe/nehe_lessons.c` so both disks use
the same scenes.
